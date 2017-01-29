/* mods.c

 This is a launcher,very simple.

 Copyright 2016-2017 Takataka

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define USE_EXECL
#define REREAD_OK

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#ifndef  USE_EXECL
	#include <signal.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//#if defined(GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION == 3)
//	#define GTK_V3
//#endif
//#if defined(GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION == 2)
//	#define GTK_V2
//#endif

#ifdef  USE_EXECL
	#define SHELL_BIN 		"/bin/sh"
#endif

#define MSG_VERSION_TEXT 	"mods version 1.3"
#define MSG_HELP_TEXT 		"modest starter\n\nhow to use...\n\nmods menu-file [menu-dir]"
#define EMSG_NOVALID_LINE 	"No valid line in the file."
#define EMSG_FILE_NOT_EXIST "The file is not exist or cannot open to read."
#define EMSG_DIR_NOT_EXIST 	"The directory is not exist or cannot access."
#define EMSG_PASS_FILENAME 	"Please pass the filename of menu as argument."

#define PREFIX_TEXT 		"text="
#define PREFIX_TEXT_LEN 	5
#define PREFIX_CMD 			"cmd="
#define PREFIX_CMD_LEN 		4

#define COL_NO1_TEXT 		"No"
#define COL_NO2_TEXT 		"Item"

#define IS_FILE 			1
#define IS_DIRECTORY 		2

#define MAX_LEN_TXT 		128
#define MAX_LEN_CMD 		256
#define MAX_LEN_MENUF 		256

#define MAX_MENU_CNT 		30
#define MAX_MENU_NEST 		10

#define MY_REREAD_NEXT 		1

#ifdef  REREAD_OK
	#define MY_REREAD_PREV 	2
#endif

/* listview 's column */
enum {
	COLUMN_ID,		/* 0 */
	COLUMN_NAME,	/* 1 */
	N_COLUMNS		/* 2 */
};

typedef struct _ListData {
	guint		no;
	gchar		name[MAX_LEN_TXT];
	gchar		cmd[MAX_LEN_CMD];
} ListData;

#define 	 ErrorMsgBox(x,y) (MyMsgBox((x),(y),GTK_MESSAGE_ERROR))
#define 	 InfoMsgBox(x,y)  (MyMsgBox((x),(y),GTK_MESSAGE_INFO))

/* prototype */
#ifndef USE_EXECL
	typedef void Sigfunc(int);
	Sigfunc 	*Signal(int , Sigfunc *);
	void 		 sigchld_handler(int);
#endif
void 		 WindowPostEvent(GdkWindow *,GdkEventType);
gboolean 	 focus_out(GtkWidget *, GdkEventKey *, gpointer);
void 		 MyMsgBox(gchar	*,GtkWindow *,GtkMessageType);
gboolean 	 check_key(GtkWidget *, GdkEventKey *, gpointer);
void 		 add_data (GtkTreeView *,int);
GtkWidget 	*create_list_model(void);
void 		 CheckAndStore(char *,int);
int 		 iReadMenuFile(char *);
void 		 MyExec(char *,char *,char *);
gboolean 	 fCheckFileDir(char *,int );
gboolean 	 ViewEnter(void);
void 		 mouse_double(GtkTreeView *,GtkTreePath *,GtkTreeViewColumn  *,gpointer );

#ifdef REREAD_OK
	int 	 iReReadMenuFile(char *,int );
#endif

/* global variable */
struct {
	ListData 	 ldata[MAX_MENU_CNT];
	GtkWidget 	*treeview;
	int 		 iCurMenuNo;
	GtkWidget	*window;
	#ifdef REREAD_OK
		char 	 MenuFile[MAX_MENU_NEST][MAX_LEN_MENUF];
		int 	 iMuneNoSave[MAX_MENU_NEST];
		int 	 iCurMenuNest;
	#else
		char 	*MySelf;
	#endif
	char 		*pMenuDir;
} g;

/* -------------------------------
 * fCheckFileDir
 * argument:pathname  : name to be checked
 *          iCheckType: type of check
 * function:check pathname is normalfile/directory or not
 * return  :TRUE : check OK
 *          FALSE: check NG
 */
gboolean fCheckFileDir(char *pathname,int iCheckType){
	struct stat sb;
	if ( pathname != NULL ){
		stat(pathname,&sb);
		if       ( iCheckType == IS_FILE )     {
			return S_ISREG(sb.st_mode);
		}else if ( iCheckType == IS_DIRECTORY ){
			return S_ISDIR(sb.st_mode);
		}
	}
	return FALSE;
}

/* -------------------------------
 * MyExec
 * argument:p1  : command
 *          p2  : argument for p1 (optional)
 *          p3  : argument for p1 (optional)
 * function:execute command
 * return  :none
 */
void MyExec(char *p1,char *p2,char *p3){
	char szCmdLine[MAX_LEN_CMD*4];
	if(( p3 != NULL )&&( p2 != NULL )&&( p1 != NULL )){
		sprintf(szCmdLine,"%s %s %s&" ,p1,p2,p3);
	}else if(( p2 != NULL )&&( p1 != NULL )){
		sprintf(szCmdLine,"%s %s&" ,p1,p2);
	}else if( p1 != NULL ){
		sprintf(szCmdLine,"%s&" ,p1);
	}else{
		return;
	}
	gtk_window_set_transient_for(GTK_WINDOW(g.window),NULL);
	#ifndef USE_EXECL
		system(szCmdLine);
	#else
		execl(SHELL_BIN,SHELL_BIN,"-c",szCmdLine,NULL);
	#endif
}

#ifndef USE_EXECL
	/* -------------------------------
	 * Signal
	 * argument:sig_no,func
	 * function:regist handler of SIGCHLD
	 * return  :handler
	 */
	Sigfunc *Signal(int sig_no, Sigfunc *func){
		struct sigaction new_act, prv_act;

		new_act.sa_handler = func;
		sigemptyset(&new_act.sa_mask);
		new_act.sa_flags = 0;
//		if( sig_no == SIGALRM ){
//			new_act.sa_flags |= SA_INTERRUPT;
//		} else {
//			new_act.sa_flags |= SA_RESTART;
//		}
		if( sig_no == SIGCHLD ){
			new_act.sa_handler = SIG_IGN; 		/* ignore signal      */
			new_act.sa_flags  |= SA_NOCLDWAIT;
		}
		sigaction(sig_no, &new_act, &prv_act);
		return (prv_act.sa_handler);
	}

	/* -------------------------------
	 * sigchld_handler
	 * argument:sig_num
	 * function:handle of sigchld (request to exit this program)
	 * return  :none
	 */
	void sigchld_handler(int sig_num){
		/* no check and request to quit */
		gtk_main_quit();
	}
#endif

/* -------------------------------
 * WindowPostEvent
 * argument:window : target window
 *          type   : type of event
 * function:post event to window ( not send )
 * return  :none
 */
void WindowPostEvent(GdkWindow *window,GdkEventType type){
	GdkEvent ev;
	ev.any.window 		= window;
	ev.any.type 		= type;
	ev.any.send_event 	= FALSE;
	gdk_event_put(&ev);
}

/* -------------------------------
 * focus_out
 * argument:widget,event,data
 * function:post GDK_DELETE to exit this program(focus-out-event)
 * return  :TRUE  :process is complete in this routine
 */
gboolean focus_out(GtkWidget *widget, GdkEventKey *event, gpointer data){
	WindowPostEvent(gtk_widget_get_window(widget),GDK_DELETE);
	return TRUE;
}

/* -------------------------------
 * MyMsgBox
 * argument:MsgText   : text to be displayed
 *          parent_win: parent window
 *          type      : type of dialog
 * function:display dialogbox
 * return  :none
 */
void MyMsgBox(gchar	*MsgText,GtkWindow *parent_win,GtkMessageType type){
	GtkWidget	*dialog;
	dialog=gtk_message_dialog_new(	parent_win,
									0,					/* setting flag          */
									type,				/* type of dialog        */
									GTK_BUTTONS_CLOSE,	/* button                */
									MsgText);			/* message               */
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if ( parent_win != NULL ) gtk_window_present(GTK_WINDOW(parent_win));
}

/* -------------------------------
 * iReadMenuFile
 * argument:FileName
 * function:read menu file
 *          check each line(store data )
 * return  :count of ldata ( -1: cannot read )
 */
int iReadMenuFile(char *FileName){
	FILE 	*MenuFile;
	int 	j;
	char 	szLine[512];
	int 	iRc;

	if ( (MenuFile = fopen(FileName, "r")) == NULL ){
		if( g.pMenuDir != NULL) {
			sprintf(szLine,"%s/%s",g.pMenuDir,FileName);
			MenuFile = fopen(szLine,"r");
		}
	}
	if ( MenuFile == NULL ){/* the file is not exist or cannot read */
		iRc=-1;
	}else{					/* the file is exist and readable */
		#ifdef REREAD_OK
			if (g.iCurMenuNo != 0 ){
				memset(g.ldata,0x00,sizeof(g.ldata));
				g.iCurMenuNo=0;
			}
		#endif
		while(( g.iCurMenuNo < MAX_MENU_CNT )&&(fgets(szLine,sizeof(szLine),MenuFile) != NULL )){
			j=strlen(szLine);
			CheckAndStore(szLine,j);
		}
		fclose(MenuFile);
		iRc=g.iCurMenuNo;
	}
	return iRc;
}

#ifdef REREAD_OK
	/* -------------------------------
	 * iReReadMenuFile
	 * argument:FileName
	 *          iType    : MY_REREAD_NEXT / MY_REREAD_PREV
	 * function:re-read menu file
	 *          clear listview
	 *          store data to listview
	 *          adjust window size
	 * return  :count of ldata ( -1: cannot read )
	 */
	int iReReadMenuFile(char *FileName,int iType){
		int iRc=iReadMenuFile(FileName);
		if ( iRc > 0 ){
			GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(g.treeview)));
			gtk_list_store_clear(store);	/* delete data in list */

			add_data(GTK_TREE_VIEW(g.treeview),iType);

			gtk_tree_view_columns_autosize(GTK_TREE_VIEW(g.treeview));	/* this is needed to adjust width */
			gtk_window_resize(GTK_WINDOW(g.window), 1, 1);				/* adjust window size */
			gtk_widget_show_all(g.window);
		}
		return iRc;
	}
#endif

/* -------------------------------
 * mouse_double
 * argument:treeview,path,col,userdata
 * function:double click handling(on treeview)
 * return  :none
 */
void mouse_double(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn  *col,gpointer userdata){
	ViewEnter();
}

/* -------------------------------
 * ViewEnter
 * argument:void
 * function:process for Enter(on treeview)
 * return  :TRUE  :process is complete in this routine
 *          FALSE :process is not done/not complete in this routine(default process needed)
 */
gboolean ViewEnter(void){
	GtkTreeSelection	*selection;
	GtkTreeModel		*model;
	gboolean			 success;
	GtkTreeIter			 iter;
	gint				 ret_no;

	selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(g.treeview));
	if (selection) {
		success=gtk_tree_selection_get_selected(selection, &model, &iter);
		if (success) {
			gtk_tree_model_get( model,&iter,COLUMN_ID  ,&ret_no,-1);
			if ( g.ldata[ret_no-1].cmd[0]== '<' ){
				char *FileName;
				FileName=&(g.ldata[ret_no-1].cmd[1]);
				while( *FileName == ' '  ) FileName++;
				if   ( *FileName != 0x00 ){
					#ifndef  REREAD_OK
						MyExec(g.MySelf,FileName,g.pMenuDir);
					#else
						char 	FName[MAX_LEN_CMD];
						int  	iRc;

						strcpy(FName,FileName);
						iRc=iReReadMenuFile(FName,MY_REREAD_NEXT);
						if ( iRc == 0 ){		/* no valid line  */
							ErrorMsgBox(EMSG_NOVALID_LINE  ,GTK_WINDOW(g.window));
						}else if (iRc <0 ){		/* file not found */
							ErrorMsgBox(EMSG_FILE_NOT_EXIST,GTK_WINDOW(g.window));
						}else{
							int j=strlen(FName);
							if ( j < MAX_LEN_MENUF ){
								if (g.iCurMenuNest >= MAX_MENU_NEST-1){
								}else{
									g.iMuneNoSave[g.iCurMenuNest-1]=ret_no-1;
									strcpy(g.MenuFile[g.iCurMenuNest],FName);
									g.iCurMenuNest++;
								}
							}
						}
					#endif
				}
			}else{
				MyExec(g.ldata[ret_no-1].cmd,NULL,NULL);
			}
			return TRUE;
		}
	}
	return FALSE;
}

/* -------------------------------
 * check_key
 * argument:widget,event,data
 * function:pressed-key handling(key_press_event)
 * return  :TRUE  :process is complete in this routine
 *          FALSE :process is not done/not complete in this routine(default process needed)
 */
gboolean check_key(GtkWidget *widget, GdkEventKey *event, gpointer data){
	#define MY_NONE 	0
	#define MY_ESC  	1
	#define MY_ENTER	2
	#define MY_NEXT 	3
	#define MY_PREV 	4
	#define MY_NUM  	5
	#define MY_TOP  	6
	#define MY_BOT  	7

	int 	iType = MY_NONE;
	int 	iNo;

	switch( event->keyval ){
		case GDK_KEY_Escape:		/* Esc key   */
		case GDK_KEY_q:				/* 'q' key   */
		case GDK_KEY_Q:				/* 'Q' key   */
		case GDK_KEY_slash:			/* '/' key   */
			iType=MY_ESC;
			break;
		case GDK_KEY_Return:		/* Enter key */
		case GDK_KEY_KP_Enter:		/* Enter key */
		case GDK_KEY_space:			/* Space key */
		case GDK_KEY_s:				/* 's'   key */
		case GDK_KEY_S:				/* 'S'   key */
		case GDK_KEY_j:				/* 'j'   key */
		case GDK_KEY_J:				/* 'J'   key */
		case GDK_KEY_at:			/* '@'   key */
		case GDK_KEY_backslash:		/* '\' key   */
			iType=MY_ENTER;
			break;
		case GDK_KEY_Up:			/* Arrow key */
		case GDK_KEY_Left:			/* Arrow key */
		case GDK_KEY_b:				/* 'b' key   */
		case GDK_KEY_B:				/* 'B' key   */
		case GDK_KEY_p:				/* 'p' key   */
		case GDK_KEY_P:				/* 'P' key   */
			if(event->state & GDK_CONTROL_MASK){	/*  Ctrl + ... */
				iType=MY_TOP;
			}else{
				iType=MY_PREV;
			}
			break;
		case GDK_KEY_Down:			/* Arrow key */
		case GDK_KEY_Right:			/* Arrow key */
		case GDK_KEY_f:				/* 'f' key   */
		case GDK_KEY_F:				/* 'F' key   */
		case GDK_KEY_n:				/* 'n' key   */
		case GDK_KEY_N:				/* 'N' key   */
			if(event->state & GDK_CONTROL_MASK){	/*  Ctrl + ... */
				iType=MY_BOT;
			}else{
				iType=MY_NEXT;
			}
			break;
		case GDK_KEY_Home:			/* Home key  */
		case GDK_KEY_t:				/* 't' key   */
		case GDK_KEY_T:				/* 'T' key   */
			iType=MY_TOP;
			break;
		case GDK_KEY_End:			/* End key  */
		case GDK_KEY_l:				/* 'l' key   */
		case GDK_KEY_L:				/* 'L' key   */
		case GDK_KEY_v:				/* 'v' key   */
		case GDK_KEY_V:				/* 'V' key   */
			iType=MY_BOT;
			break;
		default:
			if((event->keyval >= GDK_KEY_0)&&(event->keyval <= GDK_KEY_9)){	/* number key  */
				iType=MY_NUM;
				iNo=event->keyval-GDK_KEY_0; 									/* 0 - 9       */
			}else if((event->keyval >= GDK_KEY_KP_0)&&(event->keyval <= GDK_KEY_KP_9)){ /* number key  */
				iType=MY_NUM;
				iNo=event->keyval-GDK_KEY_KP_0; 								/* 0 - 9       */
			}else{
				return TRUE;
			}
	}

	if ( iType != MY_NONE ){
		GtkTreeSelection	*selection;
		GtkTreeModel		*model;
		gboolean			 success;
		GtkTreeIter			 iter;
		gint				 ret_no;
		switch(iType){
			case MY_ESC:
				#ifdef REREAD_OK
					if ( g.iCurMenuNest == 1 ){
						WindowPostEvent(gtk_widget_get_window(widget),GDK_DELETE);
					}else{
						g.iCurMenuNest--;
						iReReadMenuFile(g.MenuFile[g.iCurMenuNest-1],MY_REREAD_PREV);
					}
				#else
					WindowPostEvent(gtk_widget_get_window(widget),GDK_DELETE);
				#endif
				return TRUE;
				break;
			case MY_ENTER:
				return ViewEnter();
				break;
			case MY_NEXT:
			case MY_PREV:
				selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(g.treeview));
				if (!selection) {
					gtk_tree_selection_select_path(selection,
													gtk_tree_path_new_from_indices(0,-1));
					return TRUE;
				}else{
					success=gtk_tree_selection_get_selected(selection, &model, &iter);
					if (success) {
						gtk_tree_model_get( model,&iter,
											COLUMN_ID  ,&ret_no,
											-1); /* last */
						if ( iType == MY_PREV ){
							ret_no -= 2;
						}
						gtk_tree_selection_select_path(selection,
														gtk_tree_path_new_from_indices(ret_no,-1));
						return TRUE;
					}
				}
				break;
			case MY_TOP:
			case MY_BOT:
				selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(g.treeview));
				if (selection) {
					if ( iType == MY_TOP ){
						iNo = 1;
					}else{
						iNo = g.iCurMenuNo;
					}
					gtk_tree_selection_select_path(selection,
												gtk_tree_path_new_from_indices(iNo-1,-1));
					return TRUE;
				}
				break;
			case MY_NUM:
				selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(g.treeview));
				if (selection) {
					if(iNo==0) iNo=10; 						/* treat as 10 */
					if(event->state & GDK_MOD1_MASK){			/* Mod1(Alt) + */
						iNo+=20;
					}else if(event->state & GDK_CONTROL_MASK){	/*  Ctrl + ... */
						iNo+=10;
					}
					if ( iNo > g.iCurMenuNo ) iNo = g.iCurMenuNo;
					gtk_tree_selection_select_path(selection,
												gtk_tree_path_new_from_indices(iNo-1,-1));
					return TRUE;
				}
				break;
		}
	}
	return FALSE;
	#undef MY_NONE
	#undef MY_ESC
	#undef MY_ENTER
	#undef MY_NEXT
	#undef MY_PREV
	#undef MY_NUM
	#undef MY_TOP
	#undef MY_BOT
}

/* -------------------------------
 * add_data
 * argument:treeview
 *          iType    : MY_REREAD_NEXT / MY_REREAD_PREV
 * function:insert ldata to treeview
 * return  :none
 */
void add_data(GtkTreeView *treeview,int iType){
	GtkListStore	*store;
	GtkTreeIter		 iter;
	int				 i;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(treeview);

	store = GTK_LIST_STORE(gtk_tree_view_get_model(treeview));
//	for (i = 0; i < g.iCurMenuNo; i++) {
//		gtk_list_store_append(store, &iter);
	for (i = g.iCurMenuNo-1;i > -1; i--) {
		gtk_list_store_prepend(store, &iter); /* may be faster than append */
		gtk_list_store_set( store, &iter,
							COLUMN_ID,   g.ldata[i].no,
							COLUMN_NAME, g.ldata[i].name, -1);
	}
		/* make default selection  */
	#ifdef REREAD_OK
		if (iType==MY_REREAD_PREV){
			gtk_tree_selection_select_path(sel,
								gtk_tree_path_new_from_indices(g.iMuneNoSave[g.iCurMenuNest-1],-1));
		}else{
			gtk_tree_selection_select_path(sel,
							gtk_tree_path_new_from_indices(0,-1));
		}
	#else
		gtk_tree_selection_select_path(sel,
							gtk_tree_path_new_from_indices(0,-1));
	#endif
}

/* -------------------------------
 * create_list_model
 * argument:none
 * function:create listview
 * return  :none
 */
GtkWidget *create_list_model(void){
	GtkWidget			*treeview;
	GtkListStore		*liststore;
	GtkCellRenderer		*renderer;
	GtkTreeViewColumn	*column;

	liststore = gtk_list_store_new(N_COLUMNS,G_TYPE_UINT, G_TYPE_STRING);
	treeview  = gtk_tree_view_new_with_model(GTK_TREE_MODEL(liststore));
	g_object_unref(liststore);

	/*add 1st column*/
	renderer = gtk_cell_renderer_text_new();
	column   = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title( column, COL_NO1_TEXT);
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer,"text", COLUMN_ID, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	/*add 2nd column*/
	renderer = gtk_cell_renderer_text_new();
	column   = gtk_tree_view_column_new_with_attributes(COL_NO2_TEXT, renderer,
														"text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	g_signal_connect(G_OBJECT(treeview), "row-activated"   , G_CALLBACK(mouse_double) , NULL);

	return treeview;
}

/* -------------------------------
 * CheckAndStore
 * argument:pLine    : a line of menu-file
 *          iLen     : length of the line
 * function:check line and store into ldata
 * return  :none
 */
void CheckAndStore(char *pLine,int iLen){
	char 	*p1=pLine;

	while( *p1 == ' ' ) p1++;
	if   ( *p1 == '#' ){
		/* comment line */
	}else{
		char 	*p2;
		char 	*pText;
		int 	 iStrLenText,iStrLenCmd;

		if( memcmp(p1,PREFIX_TEXT,PREFIX_TEXT_LEN) == 0 ){
			p1+=PREFIX_TEXT_LEN;
			while( *p1 == ' ' ) p1++;
			if   ( *p1 == '"' ){
				p1++;
				p2=p1;
				while(( *p2 != '\0' )&&( *p2 != '"' )) p2++;
				if   ( *p2 == '"' ){
					iStrLenText=p2-p1;
					if( iStrLenText < MAX_LEN_TXT ){
						pText=p1;
						p1=p2+1;
						while(( *p1 == ' ' )||( *p1 == '\t' )) p1++;
						if   ( *p1 == ';' ){
							p1++;
							while(( *p1 == ' ' )||( *p1 == '\t' )) p1++;
							if   ( memcmp(p1,PREFIX_CMD,PREFIX_CMD_LEN) == 0 ){
								p1+=PREFIX_CMD_LEN;
								while(( *p1 == ' ' )||( *p1 == '\t' )) p1++;
								if   ( *p1 == '"' ){
									p1++;
									while(( *p1 == ' ' )||( *p1 == '\t' )) p1++;
									p2=pLine+iLen-1;
									while( *p2 != '"' ) p2--;
									p2--;
									while(( *p2 == ' ' )||( *p2 == '\t' )) p2--;
									if   ( p2 > p1 ){
										iStrLenCmd=p2-p1+1;
										if( iStrLenCmd < MAX_LEN_CMD ){
											g.ldata[g.iCurMenuNo].no=g.iCurMenuNo+1;
											memcpy(g.ldata[g.iCurMenuNo].name,pText,iStrLenText);
											memcpy(g.ldata[g.iCurMenuNo].cmd ,p1   ,iStrLenCmd );
											g.iCurMenuNo++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

/* -------------------------------
 * main
 * argument:'--version'    : display version
 *          '--help'/'-?'  : display help
 *          menu-file-name : file to be read
 *          directory      : directory including menu-file (optional)
 */
int main (int argc, char **argv){
	gchar	*MsgText=NULL;

//	gtk_set_locale();
	setlocale(LC_ALL, "");

	gtk_init(&argc, &argv);

	#ifndef REREAD_OK
		g.MySelf= argv[0];
	#endif

//	gtk_rc_parse(".modsrc");

	if( argc > 1 ){
		if ( strcmp(argv[1],"--version") == 0 ){
			InfoMsgBox(MSG_VERSION_TEXT,NULL);
			return EXIT_SUCCESS;
		}else if (( strcmp(argv[1],"--help") == 0 )||( strcmp(argv[1],"-?") == 0 )){
			InfoMsgBox(MSG_HELP_TEXT,NULL);
			return EXIT_SUCCESS;
		}else{
			if ( argc > 2 ){
				if ( fCheckFileDir(argv[2],IS_DIRECTORY) == TRUE ){
					g.pMenuDir=argv[2];
				}else{
					MsgText = EMSG_DIR_NOT_EXIST;
				}
			}
			if ( MsgText == NULL ){
				int iRc=iReadMenuFile(argv[1]);
				if ( iRc < 1 ){
					/* the file is not exist */
					MsgText = EMSG_FILE_NOT_EXIST;
				}else if ( iRc == 0 ){
					MsgText = EMSG_NOVALID_LINE;
				}else{
					#ifdef REREAD_OK
						int j=strlen(argv[1]);
						if ( j < MAX_LEN_MENUF ){
							strcpy(g.MenuFile[g.iCurMenuNest],argv[1]);
							g.iMuneNoSave[g.iCurMenuNest]=0;
							g.iCurMenuNest++;
						}else{
							g.iCurMenuNest=-1;
						}
					#endif
				}
			}
		}
	}else{
		MsgText = EMSG_PASS_FILENAME;
	}

	if ( MsgText != NULL ){
		ErrorMsgBox(MsgText,NULL);
		return EXIT_FAILURE;
	}

	#ifndef USE_EXECL
		Signal(SIGCHLD, sigchld_handler);
	#endif

	g.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(g.window),TRUE);

//	gtk_window_set_title(GTK_WINDOW(g.window), "mods");		/* wndow title */
	gtk_container_set_border_width(GTK_CONTAINER(g.window), 3);

	/* set signal/event routine */
	g_signal_connect(G_OBJECT(g.window), "destroy"         , G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(g.window), "key_press_event", G_CALLBACK(check_key)    , NULL);
	g_signal_connect(G_OBJECT(g.window), "focus-out-event", G_CALLBACK(focus_out)    , NULL);

//	gtk_window_set_default_size(GTK_WINDOW(g.window), 10, 10);		/* window size      */
	gtk_window_set_position(GTK_WINDOW(g.window), GTK_WIN_POS_CENTER_ALWAYS);	/* window posiotion */

	g.treeview = create_list_model();
	gtk_container_add(GTK_CONTAINER(g.window), g.treeview);

	add_data(GTK_TREE_VIEW(g.treeview),MY_REREAD_NEXT);

//	gtk_widget_realize(g.window);
		/* remove titlebar/frame ...*/
	gtk_window_set_decorated(GTK_WINDOW(g.window), FALSE);
	gtk_widget_show_all(g.window);

	gtk_window_present(GTK_WINDOW(g.window));/* need to catch key! <- gtk_window_set_skip_taskbar_hint(x,TRUE) */

	gtk_main();

	return EXIT_SUCCESS;
}