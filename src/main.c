/* main.c
 *
 * Copyright 2025 Adam Masciola
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "BAZAAR::MAIN"

#include "config.h"

#include <bge.h>
#include <glib/gi18n.h>
#include <libdex.h>

/* DOWNSTREAM PATCH (APT system updates): renderer choice, hook H13 */
#include <dlfcn.h>
#include <string.h>

#include "bz-application.h"
#include "refresh-worker.h"
#include "update-worker.h"

/* DOWNSTREAM PATCH (APT system updates): GTK renderer choice, see
 * ai-patches/DIFF.md hook H13.
 *
 * TRUE when GL only runs in software (Mesa's llvmpipe, softpipe, swrast:
 * virtual machines without 3D acceleration, broken drivers) or not at all.
 * GTK's GPU renderers then compile their shaders on the CPU, on the main
 * thread, the first time each one is needed; the Search page alone froze
 * the window for seconds ("not responding"). GTK does not tell which GL
 * driver it got, so ask GL itself, through libepoxy, which GTK loads. */
static gboolean
gl_is_software_only (void)
{
  typedef const unsigned char *(*GetStringFunc) (unsigned int name);
  static const char *const software[] = { "llvmpipe", "softpipe", "swrast", "software rasterizer", NULL };
  g_autoptr (GdkGLContext) context = NULL;
  g_autoptr (GError) local_error   = NULL;
  g_autofree char *renderer        = NULL;
  GetStringFunc   *get_string      = NULL;
  const char      *string          = NULL;
  GdkDisplay      *display         = NULL;

  display = gdk_display_get_default ();
  if (display == NULL)
    return FALSE;

  context = gdk_display_create_gl_context (display, &local_error);
  if (context == NULL || !gdk_gl_context_realize (context, &local_error))
    {
      g_debug ("No usable GL: %s", local_error != NULL ? local_error->message : "unknown error");
      return TRUE;
    }

  get_string = dlsym (RTLD_DEFAULT, "epoxy_glGetString");
  if (get_string == NULL || *get_string == NULL)
    return FALSE;

  gdk_gl_context_make_current (context);
  string   = (const char *) (*get_string) (0x1F01 /* GL_RENDERER */);
  renderer = g_ascii_strdown (string != NULL ? string : "", -1);
  gdk_gl_context_clear_current ();

  g_debug ("GL renderer: %s", renderer);
  for (guint i = 0; software[i] != NULL; i++)
    if (strstr (renderer, software[i]) != NULL)
      return TRUE;
  return FALSE;
}

/* Sets GSK_RENDERER, unless the user did, before GTK creates a renderer:
 * - "cairo" when GL runs in software only (see above);
 * - "gl" otherwise: GTK 4.23's default Vulkan renderer draws the Search page
 *   without text and with blank tiles, its GL renderer draws it correctly. */
static void
select_gsk_renderer (void)
{
  const char *renderer = NULL;

  if (g_getenv ("GSK_RENDERER") != NULL)
    return;
  if (!gtk_init_check ())
    return;

  renderer = gl_is_software_only () ? "cairo" : "gl";
  g_debug ("Using GTK's %s renderer", renderer);
  g_setenv ("GSK_RENDERER", renderer, FALSE);
}

int
main (int   argc,
      char *argv[])
{
  int result = 0;

  if (argc > 1 && g_strcmp0 (argv[1], "--version") == 0)
    {
      g_print ("%s\n", PACKAGE_VCS_VERSION);
      return 0;
    }

  dex_init ();

  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  if (argc > 1 && g_strcmp0 (argv[1], REFRESH_WORKER_CLI_OPTION) == 0)
    result = run_refresh_worker (argc, argv);
  else if (argc > 1 && g_strcmp0 (argv[1], UPDATE_WORKER_CLI_OPTION) == 0)
    result = run_update_worker (argc, argv);
  else
    {
      g_autoptr (BzApplication) app = NULL;

      /* DOWNSTREAM PATCH (APT system updates): GTK renderer choice, see
       * ai-patches/DIFF.md hook H13. Before the first window. */
      select_gsk_renderer ();

      /* Init Bazaar GTK Extensions (see ../bge/) */
      bge_init ();

      app = g_object_new (
          BZ_TYPE_APPLICATION,
          "application-id", APPLICATION_ID,
          "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
          "resource-base-path", "/id/blankonlinux/warsi",
          NULL);
      result = g_application_run (G_APPLICATION (app), argc, argv);
    }

  return result;
}
