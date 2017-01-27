/*
 * Copyright 2005 - Gustavo Iñiguez Goya
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * compilation: gcc gpavcl.c -o gpavcl `pkg-config --libs --cflags gtk+-2.0 gthread-2.0` 
 *
*/



#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>


#include <locale.h>

#if defined __GNUC__ && __GNUC__ < 3
#error This program will crash if compiled with g++ 2.x
// see the dynamic_cast bug in the gtkmm FAQ
#endif //
//#include "config.h"
/*
 *  * Standard gettext macros.
 *   */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define CARGAR_CONF 0
#define GUARDAR_CONF 1



typedef struct _GtkDialogWindow {
	GtkWidget *win;
	GtkWidget *cmd_button;
	GtkWidget *box;
	GtkWidget *label;
} DialogWindow;

typedef struct _GMailBombData {
	char *bateria;
	char *to;
	char *from;
	int heu;
	int cont_spyware, cont_virus, cont_joke, cont_hack, cont_dialer;
	int cont_des, cont_ren, cont_del, cont_sus, cont_ign, cont_inc;
	
	int stopScan;
	int running;
} Data;


typedef struct _GtkMainWindow {
	char *bateria;
	char *to;
	char *from;
	int stopScan;
	int running;
	char *conf_file;
	int cont_spyware, cont_virus, cont_joke, cont_hack, cont_dialer, cont_files;
	int cont_des, cont_ren, cont_del, cont_sus, cont_ign, cont_inc;

	GtkWidget *win;
	GtkWidget *pbar;
	GtkWidget *sbar;
	GtkWidget *cmd_enviar;
	GtkWidget *cmd_parar;
	GtkWidget *cmd_salir;
	GtkWidget *box; // Container global
	GtkWidget *vbox_izkierda; // Container de la derecha con los campos
	GtkWidget *vbox_buttons; // Container de la izquierda que contiene los botones
	GtkWidget *hbox_central; //Container que tiene vbox_derecha y hbox_buttons
	GtkWidget *hbox_informes;
	GtkWidget *label_txt;
	GtkWidget *frame;
	GtkWidget *cmd_file;
	GtkWidget *dialog_baterias;
	GtkWidget *label_baterias;
	GtkWidget *lbl_subject;
	GtkWidget *heu, *lbl_file;
	GtkAdjustment *heu_adj;
	GtkWidget *treeview;
	GtkWidget *anal_allext; // analizar todas las extensiones
	GtkWidget *anal_cmp; // analizar comprimidos
	GtkWidget *anal_heu; // checkbox, analizar con heuristico
	GtkWidget *anal_exc; // excluir estas extensiones
	GtkWidget *anal_ext; // analizar estas extensiones
	GtkWidget *lbl_cont_virus;
	GtkWidget *lbl_cont_actions;
	GtkWidget *options; // expander que muestra las opciones de deeteccion
	GtkWidget *vbox_options;
}MainWindow;

static MainWindow widgets;

static void create_window (MainWindow *mainwin);
//static void child (MainWindow *widgets);
static void *child (void *widgs);
static void on_filesel_delete_event (GtkWidget *widget, GdkEvent *event, gpointer *data);
static void delete_event (GtkWidget *widget, GdkEvent* event, MainWindow *widgets);
static gboolean destroy_event (GtkWidget *widget, MainWindow *widgets);
static void on_filesel_dialog_cancel (GtkWidget *widget, MainWindow *widgets);
static void on_filesel_dialog_ok (GtkWidget *widget, MainWindow *widgets);
static void on_cmdfile_clicked (GtkWidget *widget, MainWindow *widgets);
static void on_cmdsalir_clicked (GtkWidget *widget, MainWindow *widgets);
static void on_cmdenviar_clicked (GtkWidget *widget, MainWindow *widgets);
static void on_cmdparar_clicked (GtkWidget *widget, MainWindow *widgets);
static void salir (MainWindow *widgets);
static void config (MainWindow *widgets, int accion);
static void on_anal_heu_clicked (GtkWidget *widget, MainWindow *widgets);
static void show_message (GtkWindow *parent, GtkDialogFlags flags, GtkMessageType msg_type, GtkButtonsType buttons_type, const char *msg, ...);



int main (int argc, char *argv[])
{
#if defined(ENABLE_NLS)
   bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
   textdomain (GETTEXT_PACKAGE);
#endif //ENABLE_NLS



	widgets.stopScan = 0;
	widgets.running = 0;
	widgets.bateria = 0;
	widgets.to = 0;
	widgets.from = 0;
	widgets.win = 0;
	widgets.sbar = 0;
	widgets.pbar = 0;
	widgets.heu = 0;
	widgets.conf_file = 0;
	widgets.anal_allext=0; // analizar todas las extensiones
	widgets.anal_cmp=0; // analizar comprimidos
	widgets.anal_heu=0; // checkbox, analizar con heuristico
	widgets.anal_exc=0; // excluir estas extensiones
	widgets.anal_ext=0; // analizar estas extensiones

	widgets.to = (char *) malloc (256);
	widgets.bateria = (char *) malloc (256);
	widgets.from = (char *) malloc (256);
	/*
	widgets.conf_file = (char *) malloc (256);

	widgets.conf_file = (char *) g_get_home_dir ();
	g_print ("HOME: %s\n", widgets.conf_file);
	strncat (widgets.conf_file, "/.gmailbomb/gmailbomb.cfg", 26);
	g_print ("HOME: %s\n", widgets.conf_file);
	*/

	//config (&widgets, CARGAR_CONF);

	if ( !g_thread_supported () ){
		g_thread_init (NULL);
		gdk_threads_init ();
	}
	else{
		g_print ("Threads not supported\n");
	}
	
	gdk_threads_enter ();
	gtk_init (&argc, &argv);

	widgets.win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW(widgets.win), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW(widgets.win), 300, 500);
	//gtk_window_set_decorated (GTK_WINDOW(widgets.win), FALSE);
	//gtk_window_set_keep_above (GTK_WINDOW(widgets.win), TRUE);
	gtk_window_set_resizable (GTK_WINDOW(widgets.win), TRUE);

	widgets.box = gtk_vbox_new (FALSE, 10);
	widgets.cmd_enviar = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	
	widgets.cmd_parar = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	gtk_widget_set_sensitive (GTK_WIDGET(widgets.cmd_parar), FALSE);

	widgets.cmd_salir = gtk_button_new_from_stock (GTK_STOCK_QUIT);

	gtk_button_set_label (GTK_BUTTON(widgets.cmd_parar), "Parar");
	gtk_button_set_use_stock (GTK_BUTTON(widgets.cmd_parar), TRUE);
	
	widgets.options = gtk_expander_new ("Opciones");
	widgets.vbox_options = gtk_vbox_new (FALSE, 10);
	
	widgets.anal_allext = gtk_check_button_new_with_label ("Todas las extensiones"); 
	widgets.anal_cmp = gtk_check_button_new_with_label ("Analizar comprimidos"); 
	widgets.anal_heu = gtk_check_button_new_with_label ("Usar heuristico"); 
	widgets.anal_exc = gtk_check_button_new_with_label ("Excluir estas extensiones"); 
	widgets.anal_ext = gtk_check_button_new_with_label ("Analizar estas extensiones"); 

	widgets.label_txt = gtk_label_new ("De:");
	gtk_label_set_line_wrap (GTK_LABEL(widgets.label_txt), FALSE);
	gtk_label_set_justify (GTK_LABEL(widgets.label_txt), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL(widgets.label_txt), FALSE);

	widgets.label_baterias = gtk_label_new_with_mnemonic ("BaterÃ­a:");
	gtk_label_set_justify (GTK_LABEL(widgets.label_baterias), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL(widgets.label_baterias), TRUE);

	widgets.lbl_subject = gtk_label_new ("Asunto:");

	widgets.heu_adj = (GtkAdjustment *) gtk_adjustment_new (3.0, 0.0, 3.0, 1.0, 3.0, 3.0);
	widgets.heu = gtk_spin_button_new (widgets.heu_adj, 1.0, 0);
	widgets.lbl_file = gtk_label_new ("Fichero:");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(widgets.heu), 1.0);
	gtk_widget_set_sensitive (GTK_WIDGET(widgets.heu), FALSE);

	widgets.lbl_cont_virus = gtk_label_new ("");
	gtk_label_set_use_markup (GTK_LABEL(widgets.lbl_cont_virus), TRUE);
	widgets.lbl_cont_actions = gtk_label_new ("Desinfectados: 0\nRenombrados: 0\nBorrados: 0\nSospechosos: 0\nIgnorados: 0\nIncidencias: 0");
	gtk_label_set_use_markup (GTK_LABEL(widgets.lbl_cont_actions), TRUE);
	gtk_label_set_markup (GTK_LABEL(widgets.lbl_cont_virus), "<span weight=\"bold\">Ficheros:</span>\t0\n<span weight=\"bold\">Virus:</span>\t0\n<span weight=\"bold\">Spyware:</span>\t0\n<span weight=\"bold\">Dialers:</span>\t0\n<span weight=\"bold\">Jokes:</span>\t0\n<span weight=\"bold\">Hacking:</span>\t0");
	gtk_label_set_markup (GTK_LABEL(widgets.lbl_cont_actions), "<span weight=\"bold\">Desinfectados:</span>\t0\n<span weight=\"bold\">Renombrados:</span>\t0\n<span weight=\"bold\">Borrados:</span>\t\t0\n<span weight=\"bold\">Sospechosos:</span>\t0\n<span weight=\"bold\">Incidencias:</span>\t0");
	//widgets.lbl_cont_actions = gtk_label_new ("Desinfectados: 0\nRenombrados: 0\nBorrados: 0\nSospechosos: 0\nIgnorados: 0\nIncidencias: 0");

	widgets.vbox_buttons = gtk_vbox_new (FALSE, 10);
	widgets.vbox_izkierda = gtk_vbox_new (FALSE, 10);
	widgets.hbox_central = gtk_hbox_new (FALSE, 10); 
	widgets.hbox_informes = gtk_hbox_new (FALSE, 10); 
	widgets.pbar = gtk_progress_bar_new ();
	widgets.sbar = gtk_statusbar_new ();
	widgets.frame = gtk_frame_new ("gpavcl 0.1");
	widgets.cmd_file = gtk_button_new_from_stock (GTK_STOCK_OPEN);
	gtk_button_set_use_stock (GTK_BUTTON(widgets.cmd_file), TRUE);

	//gtk_button_set_label (GTK_BUTTON(widgets.cmd_file), "Seleccionar baterÃ­a");
	widgets.dialog_baterias = gtk_file_selection_new ("Seleccionar baterÃ­a");

	// AÃ±adimos el Vbox a la ventana
	gtk_container_add (GTK_CONTAINER(widgets.win), widgets.box);
	gtk_box_pack_start (GTK_BOX(widgets.box), widgets.frame, TRUE, TRUE, 5);

	
	// y al frame aÃ±adimos la caja para organizar los widgets
	gtk_container_add (GTK_CONTAINER(widgets.frame), widgets.hbox_central);
	// Ahora se aÃ±ade el container horizontal que contiene todo
	//gtk_box_pack_start (GTK_BOX(widgets.box), widgets.hbox_central, TRUE, TRUE, 5);
	// Al container horizontal los cotainers con los widgets
	gtk_box_pack_start (GTK_BOX(widgets.hbox_central), widgets.vbox_izkierda, TRUE, TRUE, 5);
	gtk_box_pack_start (GTK_BOX(widgets.hbox_central), widgets.vbox_buttons, FALSE, TRUE, 5);

	gtk_container_set_border_width (GTK_CONTAINER(widgets.frame), 5);

	gtk_window_set_default_size (GTK_WINDOW(widgets.win), 350, 400);
	
	g_signal_connect (G_OBJECT(widgets.win), "delete_event", G_CALLBACK(delete_event), &widgets);
	g_signal_connect (G_OBJECT(widgets.win), "destroy_event", G_CALLBACK(destroy_event), &widgets);

	// seÃ±ales "clicked" de los botones
	g_signal_connect (G_OBJECT (widgets.cmd_salir), "clicked", G_CALLBACK (on_cmdsalir_clicked), &widgets);
	g_signal_connect (G_OBJECT (widgets.cmd_enviar), "clicked", G_CALLBACK (on_cmdenviar_clicked), &widgets);
	g_signal_connect (G_OBJECT (widgets.cmd_parar), "clicked", G_CALLBACK (on_cmdparar_clicked), &widgets);
	g_signal_connect (G_OBJECT (widgets.cmd_file), "clicked", G_CALLBACK (on_cmdfile_clicked), &widgets);
	g_signal_connect (G_OBJECT (widgets.anal_heu), "clicked", G_CALLBACK (on_anal_heu_clicked), &widgets);



	// seÃ±ales "clicked" de los botones del diÃ¡logo selecciÃ³n de ficheros
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(widgets.dialog_baterias)->ok_button), "clicked", G_CALLBACK (on_filesel_dialog_ok), &widgets);
	g_signal_connect (G_OBJECT(GTK_FILE_SELECTION(widgets.dialog_baterias)->cancel_button), "clicked", G_CALLBACK (on_filesel_dialog_cancel), &widgets);
	
	//g_signal_connect_swapped (GTK_FILE_SELECTION (widgets.dialog_baterias)->ok_button, "clicked", G_CALLBACK(gtk_widget_destroy), widgets.dialog_baterias);
	//g_signal_connect_swapped (GTK_FILE_SELECTION (widgets.dialog_baterias)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), widgets.dialog_baterias);
	g_signal_connect (G_OBJECT (widgets.dialog_baterias), "delete_event", G_CALLBACK (on_filesel_delete_event), &widgets);

	
	//gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.label_txt, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.options, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_options), widgets.anal_allext, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_options), widgets.anal_cmp, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_options), widgets.anal_ext, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_options), widgets.anal_exc, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_options), widgets.anal_heu, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_options), widgets.heu, FALSE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER(widgets.options), widgets.vbox_options);
	//gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.label_txt1, FALSE, TRUE, 0);
	//gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.lbl_subject, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.cmd_file, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.label_baterias, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.lbl_file, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_izkierda), widgets.hbox_informes, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.hbox_informes), widgets.lbl_cont_virus, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(widgets.hbox_informes), widgets.lbl_cont_actions, FALSE, TRUE, 0);
	

	gtk_box_pack_start (GTK_BOX(widgets.hbox_central), widgets.vbox_buttons, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX(widgets.box), widgets.pbar, FALSE, FALSE, 5);
	
	gtk_box_pack_start (GTK_BOX(widgets.box), widgets.sbar, FALSE, FALSE, 5);
	/*
	 _______
	|_______|
	|_______|
	|_______|
	|	|
	*/

	// botones empaquetados en horizontal
	gtk_box_pack_start (GTK_BOX(widgets.vbox_buttons), widgets.cmd_enviar, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_buttons), widgets.cmd_parar, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX(widgets.vbox_buttons), widgets.cmd_salir, FALSE, FALSE, 5);
	/*
	 _________
	|____|____|
	*/

	gtk_widget_show_all (widgets.win);

	gtk_main ();
	
	gdk_threads_leave ();

return 0;
}

static void show_message (GtkWindow *parent, GtkDialogFlags flags, GtkMessageType msg_type, GtkButtonsType buttons_type, const char *msg, ...)
{

	gdk_threads_enter ();
	GtkWidget *dig = 0;
	dig = gtk_message_dialog_new (GTK_WINDOW(parent), flags, msg_type, buttons_type, msg);
	gtk_dialog_run (GTK_DIALOG(dig));
	gtk_widget_destroy (dig);
	gdk_threads_leave ();
}


// función ofuscada... virgen santísima

static void config (MainWindow *widgets, int accion)
{
char *home=0;
FILE *f=0;
char *linea=NULL;
size_t len=0;
ssize_t read=0;
int ret=0;


//g_print ("PID: %d - GID: %d\n", (int)getuid(), (int)getgid());

//widgets->conf_file = (char *) malloc (256);

// Guardamos el nombre del fichero, de forma que si al intentar abrirlo da error, probaremos a crear el directorio

g_print ("FILE: %s\n", widgets->conf_file);

if (accion == CARGAR_CONF){
	f = fopen (widgets->conf_file, "r");
	g_print ("\tCargar conf: %s\n", widgets->conf_file);
}
else if (accion == GUARDAR_CONF){
	f = fopen (widgets->conf_file, "w+");
}

// Si no se puede abrir el fichero de configuración, entre 100^6 de posibilidades puede querer decir que el directorio no existe, creémoslo.

if (f == NULL){
	if (mkdir (home, 0666) == 0){
		g_print ("Directorio de configuración creado\n");
			
		if ((ret = chown (home, getuid(), getgid())) == -1){
			g_print ("Error cambiando los permisos al usuario actual (%d, %d)", getuid(), getgid());
		}
		ret = chmod (home, S_IRUSR | S_IWUSR | S_IXUSR | S_IXOTH | S_IROTH | S_IRGRP | S_IXGRP);
		snprintf (widgets->conf_file, 256, "%s/.gpavcl/gpavcl.cfg", g_get_home_dir());
	}
	else{
		g_print ("Falló la creación del directorio de configuración\n");
	}
}
else{
	if (accion == CARGAR_CONF){
		while ((read = getline (&linea, &len, f)) != -1){
			if (strstr(linea, "#")) continue;
			if (strstr(linea, " ")) continue;
			if (strstr(linea, ";")) continue;
			if (strstr(linea, "=")){
				g_print ("\t-- %s\n", strstr (linea, "="));
			}
			printf ("line conf: %s\n", linea);
		}
	}
	else if (accion == GUARDAR_CONF){
		fprintf (f, "bateria=%s\n", (char *)gtk_label_get_text (GTK_LABEL(widgets->label_baterias)));
	}
}

if (linea)
	free (linea);

if (f) fclose (f); f = 0;
if (home) free (home); home = 0;
//free (conf_file); conf_file = 0;

}


//static void child (MainWindow *widgets)
static void *child (void *widgs)
{
GTimer *time;
gulong microsec;
FILE *f=0, *fp=0;
char *buf_tmp=NULL;
char label_txt[50]={0};
char lbl_txt_cont[1024]={0};
char lbl_txt_cont1[1024]={0};
size_t len=0;
ssize_t read;
char **mails=0;
char **file_name=0;
double fraction=0.00;
int x=0;
double total_mails=0;
char *cmd=0;
char progress_text[10]={0};
struct rusage usage;
struct rlimit limit;

MainWindow *widgets = (MainWindow *) widgs;
	widgets->cont_files = widgets->cont_spyware = widgets->cont_virus = widgets->cont_joke = widgets->cont_hack = widgets->cont_dialer = 0;
	widgets->cont_des = widgets->cont_ren = widgets->cont_del = widgets->cont_sus = widgets->cont_ign = widgets->cont_inc = 0;

fp = fopen ("/opt/pavcl/usr/bin/pavcl", "r");

if (fp == NULL){
	printf ("No existe la pavcl\n");
	gdk_threads_enter ();
	gtk_statusbar_push (GTK_STATUSBAR(widgets->sbar), 0, "No existe la pavcl");
	gdk_threads_leave ();
	show_message (GTK_WINDOW(widgets->win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "No existe la pavcl");
	widgets->running = 0;
	widgets->stopScan = 1;
}
else{
	cmd = (char *) malloc (1024);
	sprintf (cmd, "/opt/pavcl/usr/bin/pavcl -clv -heu:1 -aex -cmp -auto %s", widgets->bateria);

	if ((f = popen (cmd, "r")) != NULL)
	{
		time = g_timer_new ();
		g_timer_start (time);

		gdk_threads_enter ();
		gtk_widget_set_sensitive (GTK_WIDGET(widgets->cmd_enviar), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(widgets->cmd_parar), TRUE);
		gtk_statusbar_push (GTK_STATUSBAR(widgets->sbar), 0, "");
		gdk_threads_leave ();

		while ((read = getline(&buf_tmp, &len, f)) != -1)
		{
			//getrusage ((int)getpid(), &usage);
			//printf ("resident size max allowed: %d - seconds: %d\n", (int)usage.ru_maxrss, (int)usage.ru_utime.tv_sec);
			//printf ("Line: %s\n", buf_tmp);
			if (widgets->stopScan == 1) break;
		
			if (strstr (buf_tmp, "MAILS")){
				mails = g_strsplit (buf_tmp, ":", 3);
				if (mails[1])
					total_mails = atoi (mails[1]);

				if (mails)
					g_strfreev (mails);
			}
			if (!strstr(buf_tmp, "Found")){
				//printf ("Read it!: %s\n", buf_tmp);
				if (total_mails > 0){
					x++;
					fraction = ((x * 100) / total_mails) / 100;
					sprintf (progress_text, "%2.f %%", fraction * 100);
					//printf ("FR: %2.f\n", fraction);
					gdk_threads_enter ();
					gtk_label_set_text (GTK_LABEL(widgets->lbl_file), buf_tmp);
					gtk_progress_bar_set_text (GTK_PROGRESS_BAR(widgets->pbar), progress_text);
					gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(widgets->pbar), fraction);
					gdk_threads_leave ();
				}
				else{
					gdk_threads_enter ();
					//printf ("1: %d - 2: %d\n", buf_tmp[strlen(buf_tmp) - 1], buf_tmp[strlen(buf_tmp) - 2]);
					if (buf_tmp[strlen(buf_tmp) - 1] == 13) buf_tmp[strlen(buf_tmp) - 1] = '\n';
					if (strstr(buf_tmp, "/")){
						file_name = g_strsplit (strstr (buf_tmp, "/"), "\\", 2);
						
						strncpy (label_txt, file_name[0], 30);
						if (g_timer_elapsed (time, &microsec) < 0.20)
							gtk_label_set_text (GTK_LABEL(widgets->lbl_file), label_txt);
						
						//if (!strstr(buf_tmp, ".:"))
						//	printf ("File: %s\n", buf_tmp);

						widgets->cont_files++;
						if (file_name)
							g_strfreev (file_name);
					}
					gtk_progress_bar_pulse (GTK_PROGRESS_BAR (widgets->pbar));
					gdk_threads_leave ();
				}
			}
			if (strstr(buf_tmp, "Found spyware")){
				widgets->cont_spyware++;
			}
			else if (strstr(buf_tmp, "Found hacking tool")){
				widgets->cont_hack++;
			}
			else if (strstr(buf_tmp, "Found virus")){
				widgets->cont_virus++;
			}
			else if (strstr(buf_tmp, "Found joke")){
				widgets->cont_joke++;
			}
			else if (strstr(buf_tmp, "Found dialer")){
				widgets->cont_dialer++;
			}
			
			if (strstr(buf_tmp, "Virus disinfected")){
				widgets->cont_des++;
			}
			else if (strstr(buf_tmp, "File deleted")){
				widgets->cont_del++;
			}
			else if (strstr(buf_tmp, "File renamed")){
				widgets->cont_ren++;
			}
			else if (strstr(buf_tmp, "Ignored")){
				widgets->cont_ign++;
			}
			else if (strstr(buf_tmp, "Virus could not be disinfected")){
				widgets->cont_inc++;
			}
			else if (strstr(buf_tmp, "Error scanning file")){
				widgets->cont_inc++;
			}
			else if (strstr(buf_tmp, "Suspicious file")){
				widgets->cont_sus++;
			}
			if (g_timer_elapsed (time, &microsec) > 0.10){
				sprintf (lbl_txt_cont, "<span weight=\"bold\">Ficheros:</span>\t%d\n<span weight=\"bold\">Virus:</span>\t%d\n<span weight=\"bold\">Spyware:</span>\t%d\n<span weight=\"bold\">Dialers:</span>\t%d\n<span weight=\"bold\">Jokes:</span>\t%d\n<span weight=\"bold\">Hacking:</span>\t%d", widgets->cont_files, widgets->cont_virus, widgets->cont_spyware, widgets->cont_dialer, widgets->cont_joke, widgets->cont_hack);
				sprintf (lbl_txt_cont1, "<span weight=\"bold\">Desinfectados:</span>\t%d\n<span weight=\"bold\">Renombrados:</span>\t%d\n<span weight=\"bold\">Borrados:</span>\t\t%d\n<span weight=\"bold\">Sospechosos:</span>\t%d\n<span weight=\"bold\">Incidencias:</span>\t%d", widgets->cont_des, widgets->cont_ren, widgets->cont_del, widgets->cont_sus, widgets->cont_inc);
				gdk_threads_enter ();
				gtk_label_set_markup (GTK_LABEL(widgets->lbl_cont_virus), lbl_txt_cont);
				gtk_label_set_markup (GTK_LABEL(widgets->lbl_cont_actions), lbl_txt_cont1);
				gdk_threads_leave ();
				g_timer_reset (time);
			}
			//else{
				//printf ("Tiempo: %.2f\n", g_timer_elapsed (time, &microsec));
			//}
			gdk_threads_enter ();
			while (gtk_events_pending())
				gtk_main_iteration ();

			gdk_flush ();
			gdk_threads_leave ();
			
			//printf ("Tiempo: %.2f\n", g_timer_elapsed (time, &microsec));
		}
		if (buf_tmp){
			free (buf_tmp);
			buf_tmp=0;
		}
		gdk_threads_enter ();
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(widgets->pbar), 0.00);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR(widgets->pbar), "");
		gtk_widget_set_sensitive (GTK_WIDGET(widgets->cmd_enviar), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(widgets->cmd_parar), FALSE);
		gtk_statusbar_push (GTK_STATUSBAR(widgets->sbar), 0, "Analisis finalizado");
		gtk_label_set_text (GTK_LABEL(widgets->lbl_file), "Fichero:");
	
		gdk_flush ();
		gdk_threads_leave ();

		pclose (f);
		widgets->running = 0;
		widgets->stopScan = 0;
		
		sprintf (lbl_txt_cont1, "<span weight=\"bold\">Informe del analisis</span>\n\n<span weight=\"bold\">Desinfectados:</span>\t%d\n<span weight=\"bold\">Renombrados:</span>\t%d\n<span weight=\"bold\">Borrados:</span>\t\t%d\n<span weight=\"bold\">Sospechosos:</span>\t%d\n<span weight=\"bold\">Incidencias:</span>\t%d", widgets->cont_des, widgets->cont_ren, widgets->cont_del, widgets->cont_sus, widgets->cont_inc);

		g_timer_stop (time);
		g_timer_destroy (time);
		//gdk_threads_enter ();
		show_message (GTK_WINDOW(widgets->win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Analisis finalizado");
		//gdk_threads_leave ();
	}
	else{
		gdk_threads_enter ();
		show_message (GTK_WINDOW(widgets->win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "No se ha podido analizar el directorio");
		gdk_threads_leave ();
	}
	if (cmd){
		free (cmd);
		cmd = 0;
	}
}
if (fp) fclose (fp);


return widgets;
}

static void on_filesel_delete_event (GtkWidget *widget, GdkEvent *event, gpointer *data)
{
	gtk_widget_hide (widget);
}

static void delete_event (GtkWidget *widget, GdkEvent* event, MainWindow *widgets)
{
	salir (widgets);
}
	
static gboolean destroy_event (GtkWidget *widget, MainWindow *widgets)
{
	if (widgets){
		salir (widgets);
	}
	return TRUE;
}

static void salir (MainWindow *widgets)
{
	if (widgets){
		//config (widgets, GUARDAR_CONF);

		widgets->stopScan = 1;
		widgets->running = 0;

		if (widgets->to) free (widgets->to); widgets->to = 0;
		if (widgets->from) free (widgets->from); widgets->from = 0;
		if (widgets->bateria) free (widgets->bateria); widgets->bateria = 0;
		if (widgets->conf_file) free (widgets->conf_file); widgets->conf_file = 0;
		g_print ("Widgets: %p - To: %p - From: %p - Bat: %p\n", widgets, widgets->to, widgets->from, widgets->bateria);
		gtk_widget_destroy (widgets->dialog_baterias);
	}
	else{
		printf ("No existe el objeto \"widgets\", no se libera memoria\n");
	}
	gtk_main_quit ();
}

static void on_filesel_dialog_cancel (GtkWidget *widget, MainWindow *widgets)
{
	if (widgets != 0)
		gtk_widget_hide (widgets->dialog_baterias);
	else
		g_print ("Problemas arrancando el diÃlogo de ficheros");
}

static void on_filesel_dialog_ok (GtkWidget *widget, MainWindow *widgets)
{
	if (widgets != 0){
		widgets->bateria = (char *)gtk_file_selection_get_filename (GTK_FILE_SELECTION (widgets->dialog_baterias));
		//g_print ("Bateria: %s\n", widgets->bateria);
		gtk_widget_hide (widgets->dialog_baterias);
		gtk_label_set_text (GTK_LABEL(widgets->label_baterias), widgets->bateria);
	}
	else{
		g_print ("Problemas obteniendo los datos, no existe el objeto \"widgets\".");
	}
}

static void on_anal_heu_clicked (GtkWidget *widget, MainWindow *widgets)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widgets->anal_heu)))
		gtk_widget_set_sensitive (GTK_WIDGET(widgets->heu), TRUE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET(widgets->heu), FALSE);
}

static void on_cmdfile_clicked (GtkWidget *widget, MainWindow *widgets)
{
	gtk_widget_show (widgets->dialog_baterias);
}

static void on_cmdsalir_clicked (GtkWidget *widget, MainWindow *widgets)
{
	//g_print ("Salimos!\n");
	if (widgets){
		salir (widgets);
	}
	else{
		printf ("widgets: %p\n", widgets);
	}
}

static void on_cmdparar_clicked (GtkWidget *widget, MainWindow *widgets)
{
	if (widgets){
		widgets->stopScan = 1;
		widgets->running = 0;
	}
}

static void on_cmdenviar_clicked (GtkWidget *widget, MainWindow *widgets)
{
	GThread *ChildThread;
	GError *error=NULL;
	if (strcmp (widgets->to, "") == 0){
		strcpy(widgets->to, "ga");
	}

	if (strcmp (widgets->from, "") == 0){
		strcpy(widgets->from, "manuÃ©");
	}

	if (widgets->bateria == 0){
		//g_print ("Label bats: %s\n", gtk_label_get_label(GTK_LABEL(widgets->label_baterias)));
		if (gtk_label_get_label(GTK_LABEL(widgets->label_baterias))){
			widgets->bateria = (char *) gtk_label_get_label(GTK_LABEL(widgets->label_baterias));
		}
		else{
			strcpy(widgets->bateria, "/tmp/");
		}
	}
	//g_print ("Bateria: %s\n", widgets->bateria);
	if (widgets->running == 0){
		widgets->running = 1;
		//child (widgets);
		//int i=0;
		//for (i=0;i < (int)gtk_spin_button_get_numeric;i++){
		if (( ChildThread = g_thread_create ((GThreadFunc)child, (void *)widgets, FALSE, &error)) == NULL){
			g_print ("GThreadCreate failed!\nMsg: %s\n", error->message);
			g_error_free (error);
		}
		//}
		//g_print ("thread: %s\n", g_thread_join (ChildThread));
		//g_spawn_async (".", , NULL, G_SPAWN_SEARCH_PATH, );
	}
	else{
		printf ("The scan IS RUNNING, be patience!\n");
	}
}

static gboolean dialog_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer *data)
{
	return FALSE;
}

static void dialog_delete_event (GtkWidget *widget, GdkEvent *event, gpointer *data)
{
	gtk_widget_hide (widget);
}

static void on_dialog_button_clicked (GtkWidget *widget, gpointer *data)
{
	//g_print ("Button clicked\n");
}

static void create_window (MainWindow *mainwin)
{
	DialogWindow dialog;
	
	dialog.win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	dialog.cmd_button = gtk_button_new_with_label ("Ver resultado");
	dialog.box = gtk_vbox_new (FALSE, 0);
	dialog.label = gtk_label_new_with_mnemonic ("Resultado:");

	gtk_window_set_default_size (GTK_WINDOW(dialog.win), 300, 400);
	g_signal_connect (G_OBJECT (dialog.win), "delete_event", G_CALLBACK(dialog_delete_event), NULL);
	g_signal_connect (G_OBJECT (dialog.win), "destroy_event", G_CALLBACK(dialog_destroy_event), NULL);
	g_signal_connect (G_OBJECT (dialog.cmd_button), "clicked", G_CALLBACK(on_dialog_button_clicked), NULL);

	gtk_container_add (GTK_CONTAINER(dialog.win), dialog.box);

	gtk_box_pack_start (GTK_BOX(dialog.box), dialog.label, FALSE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(dialog.box), dialog.cmd_button, FALSE, TRUE, 2);
	gtk_widget_show_all (dialog.win);
}