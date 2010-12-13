/*
Copyright (c) 2008, Skyler Kehren
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of 
	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of 
	conditions and the following disclaimer in the documentation and/or other materials provided 
	with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to endorse 
	or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "SnarlInterface.h"
#include "config.h"
#include <map>
#include <strsafe.h>

#pragma comment(lib, "../../../shared/shared.lib")

using namespace Snarl::V41;
using namespace pfc;

DECLARE_COMPONENT_VERSION(
		"foo_snarl",
		"0.2.0", 
		"Snarl notification interface for Foobar2000\n"
		"Developed by: Skyler Kehren (Pyrodogg)\n"
		"Contributions by:Max Battcher\n"
		"foosnarl at pyrodogg.com\n"
		"Copyright (C) 2008-2010 Skyler Kehren\n"
		"Released under BSD License");

const LPCSTR foobarIcon = "C:\\Program Files\\foobar2000\\foobar2000.exe,105";

SnarlInterface sn;
HWND hwndFooSnarlMsg;
std::map<int,char *> FSMsgClassDecode;
LONG32 lastClassMsg[4] = {0,0,0,0};
LONG32 lastMsg = 0;
int FSLastMsgClass = 0;
int FSMsgClassCount = 4;
enum FSMsgClass : int {
	Stop = 0,
	Play,
	Pause,
	Seek
};

//enum FSMsgAction : int {
//	Back = 1,
//	Next,
//	Stop,
//};


LRESULT CALLBACK WndProcFooSnarl(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch(message)
	{
	case WM_SHOWWINDOW:
		if(wParam == TRUE){
			standard_commands::main_activate();
		}
	case WM_CLOSE:
		DestroyWindow(hwnd);
	case WM_USER:
		switch(wParam)
		{
		case SnarlEnums::NotificationClicked:
			{
				static_api_ptr_t<ui_control>()->activate();
				return 0;
			}
		case SnarlEnums::NotificationAck:
			{
				return 0;
			}
		case SnarlEnums::NotificationTimedOut:
			{
				return 0;
			}
		default:
			int action = HIWORD(wParam);
			switch(action){
			case 1:
				//Back action
				static_api_ptr_t<playback_control>()->start(playback_control::track_command_prev,false);
				return 0;
			case 2:
				//Next action
				static_api_ptr_t<playback_control>()->start(playback_control::track_command_next,false);
				return 0;
			case 3:
				//Stop
				static_api_ptr_t<playback_control>()->stop();
				return 0;
			}
		}
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

class FooSnarl : public initquit, public play_callback{ 
public:

// initquit metods
 void on_init()
{
	string8 image;
	static_api_ptr_t<play_callback_manager> playCBM;
	static_api_ptr_t<ui_control> uiMain;
	WNDCLASSEX wcex;
	
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style= 0;
	wcex.lpfnWndProc = (WNDPROC) WndProcFooSnarl;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = core_api::get_my_instance();
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.hIconSm = NULL;
	wcex.lpszClassName = L"FooSnarlMsg";

	FSMsgClassDecode[FSMsgClass::Play] = "Play";
	FSMsgClassDecode[FSMsgClass::Pause] = "Pause";
	FSMsgClassDecode[FSMsgClass::Stop] = "Stop";
	FSMsgClassDecode[FSMsgClass::Seek] = "Seek";

	RegisterClassEx(&wcex);
	
	//Snippet for testing Message Window failures
	//LPTSTR lpszFunction = TEXT("RegisterClassEx");
	//	LPVOID lpMsgBuf;
	//	LPVOID lpDisplayBuf;
	//	DWORD dw = GetLastError(); 

	//	FormatMessage(
	//		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
	//		FORMAT_MESSAGE_FROM_SYSTEM |
	//		FORMAT_MESSAGE_IGNORE_INSERTS,
	//		NULL,
	//		dw,
	//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	//		(LPTSTR) &lpMsgBuf,
	//		0, NULL );

	//	// Display the error message and exit the process

	//	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
	//		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
	//	StringCchPrintf((LPTSTR)lpDisplayBuf, 
	//		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
	//		TEXT("%s failed with error %d: %s"), 
	//		lpszFunction, dw, lpMsgBuf); 
	//	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	//	LocalFree(lpMsgBuf);
	//	LocalFree(lpDisplayBuf);

	hwndFooSnarlMsg = CreateWindowEx(0,L"FooSnarlMsg",L"FooSnarl Msg",0,0,0,0,0,HWND_MESSAGE,0,0,0);

	//sn.EZNotify("","HWND",(LPCSTR)hwndFooSnarlMsg,5,0,0,0,0);

		//Register Playcallback module
	playCBM->register_callback(this,flag_on_playback_new_track | 
		flag_on_playback_dynamic_info | 
		flag_on_playback_dynamic_info_track | 
		flag_on_playback_pause | 
		flag_on_playback_stop,true);
	
	//Register Foobar2000 with Snarl
	if(sn.IsSnarlRunning()){
		//Display appropriate message depending on registration state with Snarl
		if(sn.RegisterApp("Foobar2000","Foobar2000",foobarIcon,hwndFooSnarlMsg,WM_USER,SnarlEnums::AppHasAbout)!=0){
			FSRegisterClass(Play);
			FSRegisterClass(Pause);
			FSRegisterClass(Stop);
			FSRegisterClass(Seek);
		}else{
			popup_message::g_show("FooSnarl:Unable to register with Snarl","",popup_message::icon_error);
		}

		if(hwndFooSnarlMsg == NULL){
			popup_message::g_show("FooSnarl:Unable to create message window","",popup_message::icon_error);
		}
	} else { 
		/*Snarl not running */
		//popup_message::g_show("Popup Test","Test",popup_message::icon_information);
	}
	
}

 void FSRegisterClass(int intClass){
	 if(sn.AddClass(FSMsgClassDecode[intClass],FSMsgClassDecode[intClass],true) == 0){
		 /*LPSTR error = "Unable to register ";
		 LPSTR szClass = FSMsgClassDecode[intClass];
		 error = strcat(error,szClass);
		 error = strcat(error," class");
		 popup_message::g_complain("FooSnarl",error);*/
		 popup_message::g_complain("FooSnarl","Unable to register class " + intClass);
	 }
 }

 void on_quit()
{
	//Unregister playcallback
	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
	
	//Unregister foosnarl
	if(sn.RemoveAllClasses(false)==0){
		//Failed to remove registered classes
		sn.EZNotify("","Error","FooSnarl failed to remove registered classes",10,foobarIcon,0,0,0);
	}

	if(sn.UnregisterApp()==0){
		//FooSnarl failed to unregister
		sn.EZNotify("","Error","FooSnarl failed to unregister with Snarl",10,foobarIcon,0,0,0);
	};

	lastSong = 0;
	DestroyWindow(hwndFooSnarlMsg);
	UnregisterClass(L"FooSnarlMsg",core_api::get_my_instance());
}

protected:
metadb_handle_ptr lastSong;

void on_playback_event(int alertClass){
	static_api_ptr_t<playback_control> pc;
	metadb_handle_ptr handle;
	string8 format;
	service_ptr_t<titleformat_object> script;
	string_formatter text;
	string snarl_title;
	string snarl_msg;
	string snarl_icon;
	WCHAR icon_check[MAX_PATH];
	long snarl_time;

	if(pc->get_now_playing(handle)){
		lastSong.copy(handle);
	} else {	
			handle.copy(lastSong);
	}
	
	//Process title format string for message body
	g_advconfig_string_format.get_static_instance().get_state(format);
	static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);
	pc->playback_format_title_ex(handle, NULL, text, script, NULL, play_control::display_level_titles);
	snarl_msg = text.toString();

	//Process title format string for message title
	g_advconfig_string_title_format.get_static_instance().get_state(format);
	static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);
	pc->playback_format_title_ex(handle, NULL, text, script, NULL, play_control::display_level_titles);
	snarl_title = text.toString();

	//Process title format string for message icon location
	g_advconfig_icon.get_static_instance().get_state(format);
	static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);
	pc->playback_format_title_ex(handle, NULL, text, script, NULL, play_control::display_level_titles);
	snarl_icon = text.toString();
	MultiByteToWideChar(CP_UTF8,0,snarl_icon.get_ptr(),strlen(snarl_icon.get_ptr())+1,icon_check,sizeof(icon_check)/sizeof(icon_check[0]));
		
	//Test for existance of folder.jpg picture file
	DWORD attrib = GetFileAttributes(icon_check);
	if((snarl_icon.get_ptr()=="" )||(0xFFFFFFFF == attrib)){
		snarl_icon = foobarIcon;
	}

	//Get display timeout from user settings. If invalid, send error. Shouldn't happen max and min are set. 
	snarl_time = (long) g_advconfig_time.get_static_instance().get_state_int();
	if((snarl_time==NULL)||(snarl_time==0)){
		snarl_time = 5;
		snarl_title = "ERROR";
		snarl_msg = "Set valid display time in settings";
	}


	//Send Snarl Message
	if(FSLastMsgClass != alertClass){
		sn.Hide(sn.GetLastMsgToken());
	}
	FSLastMsgClass = alertClass;

	if(sn.IsVisible(lastClassMsg[alertClass]) == -1){
		//Update existing message from same class
		sn.EZUpdate(lastClassMsg[alertClass],snarl_title.get_ptr(),snarl_msg.get_ptr(),snarl_time,snarl_icon.get_ptr());
	} else {
		//Create new message
		sn.EZNotify(FSClass(alertClass),snarl_title.get_ptr(),snarl_msg.get_ptr(),snarl_time,snarl_icon.get_ptr(),0,0,0);
		if(sn.GetLastMsgToken() != 0){
			FSAddActions();
		}
		lastClassMsg[alertClass] = sn.GetLastMsgToken();
	}	

}

LONG32 FSAddActions(){
	sn.AddAction(sn.GetLastMsgToken(),"Back",NULL);
	sn.AddAction(sn.GetLastMsgToken(),"Next",NULL);
	sn.AddAction(sn.GetLastMsgToken(),"Stop",NULL);
	return 0;
}

public:
// play_callback methods
void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {}
void on_playback_new_track(metadb_handle_ptr p_track) {
		on_playback_event(FSMsgClass::Play);
}
void  on_playback_stop(play_control::t_stop_reason p_reason) {
	if(p_reason == play_control::stop_reason_eof || p_reason == play_control::stop_reason_user){
		on_playback_event(Stop);
	}
}
void  on_playback_seek(double p_time) {}
void  on_playback_pause(bool p_state) {
	if(p_state == true){
		on_playback_event(FSMsgClass::Pause);
	} else {
		on_playback_event(FSMsgClass::Play);
	}
}
void  on_playback_edited(metadb_handle_ptr p_track) {}
void  on_playback_dynamic_info(const file_info & p_info) {}
void  on_playback_dynamic_info_track(const file_info &p_info) {}
void  on_playback_time(double p_time) {}
void  on_volume_change(float p_new_val) {}
LPSTR FSClass(int intclass){
	switch(intclass){
		case FSMsgClass::Play:
			return "Play";
			break;
		case FSMsgClass::Pause:
			return "Pause";
			break;
		case FSMsgClass::Stop:
			return "Stop";
			break;
		case FSMsgClass::Seek:
			return "Seek";
			break;
	}
}

};

//Register initquit
static initquit_factory_t< FooSnarl > foo0;
