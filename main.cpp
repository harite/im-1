#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
//#include <mem.h>



#include "id.h"


#include "sock.h"
#include "wave.h"
//#include "global.cpp"

#define VER 2
//  VER  
extern HANDLE   gRecvThread;
extern HANDLE   gSendThread;

extern DWORD    gdwRecvPid;
extern DWORD    gdwSendPid;

DWORD  WINAPI RecvThread(LPVOID);
DWORD  WINAPI SendThread(LPVOID);
int SockInit(void);

void Paint(char * str)
{ 
     #if VER<2
     // VER 2 ��ǰ�汾 û�м������ֽ��棬���� PAINT ���������Ϣ ��2 �Ժ��ֹ�˺��� 
     if(str==NULL){
                   Paint("Null String ");
                   return ;
                   }
     if(nPaintNum > 10){
                  for(int i=0;i<nPaintNum-1;i++){
                          cPaint[i]=cPaint[i+1];
                          }
                  nPaintNum--;
                  }
     cPaint[nPaintNum++]=str;
     SendMessage(ghwnd,WM_PAINT,0,0);
     #endif
}



void __cdecl Report(char *fmt,...)
{
     // ��ʽ���ַ�������ͨ����׼��Message Box  ��ʽ ������� 
     char err[256];
     va_list vls;
     va_start(vls,fmt);
     _vsnprintf(err,256,fmt,vls);
     va_end(vls);
     MessageBox(NULL,err,"Error :  report by Report()",MB_OK|MB_ICONERROR);

}


Socket   gListenSock;
Socket   gClient    ;
Socket   gConn      ;
Socket   gRecvSock  ;

WaveIn   gWaveIn    ;
WaveOut  gWaveOut   ;

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

SocketInit sockinit;

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)

{
    HWND hwnd;              
    MSG messages;            
    WNDCLASSEX wincl;        
    
    ghInst=hThisInstance;
    
    
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;     
    wincl.style = CS_DBLCLKS;                
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 
    wincl.cbClsExtra = 0;                      
    wincl.cbWndExtra = 0;                     
    
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    
    if (!RegisterClassEx (&wincl))
        return 0;

    
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "My Record",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           405,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );
           
    
    ShowWindow (hwnd, nFunsterStil);
    ghwnd=hwnd;
    
    while (GetMessage (&messages, NULL, 0, 0))
    {
        
        TranslateMessage(&messages);
        
        DispatchMessage(&messages);
    }

    
    return messages.wParam;
}

int  AddShowText(char * show)
{
    
     // ����ʾ ���� ��Ϣ�� �༭���� ��ʾ��Ϣ 
     
     char * temp;
     int    len ;
     len=GetWindowTextLength(GetDlgItem(ghwnd,ID_SHOW));
     temp=(char*)malloc(len+strlen(show)+3);
     GetWindowText(GetDlgItem(ghwnd,ID_SHOW),temp,len+1);
     strcat(temp,"\r\n");
     strcat(temp,show);
    
     SetWindowText(GetDlgItem(ghwnd,ID_SHOW),temp);  
}
 
int  OnSendText()
{
     char message[256];
     GetWindowText(GetDlgItem(ghwnd,ID_INPUT),message,256);
     if(gConn.Send(message,strlen(message)+1)==0){
             FormatError("error when send the message ,Error Code : %d ",WSAGetLastError());
             return 0;
             }
     char * temp;
     int    len ;
     len=GetWindowTextLength(GetDlgItem(ghwnd,ID_SHOW));
     temp=(char*)malloc(len+strlen(message)+3+6);
     GetWindowText(GetDlgItem(ghwnd,ID_SHOW),temp,len+1);
     strcat(temp,"\r\n");
     strcat(temp,"[send]");
     strcat(temp,message);
    
     SetWindowText(GetDlgItem(ghwnd,ID_SHOW),temp);              
     
     SetWindowText(GetDlgItem(ghwnd,ID_INPUT)," " );
	 return 1;
}

int OnRecvText()
{
    char message[256];
    
    if(gConn.Recv(message,256)==SOCKET_ERROR){
             FormatError("error when recv the text ,Error Code : %d ",WSAGetLastError());
             return 0;
             }
    char * temp;
    int    len ;
    len=GetWindowTextLength(GetDlgItem(ghwnd,ID_SHOW));
    temp=(char*)malloc(len+strlen(message)+3+6);
    GetWindowText(GetDlgItem(ghwnd,ID_SHOW),temp,len+1);
    strcat(temp,"\r\n");
    strcat(temp,"[recv]");
    strcat(temp,message);
    
    SetWindowText(GetDlgItem(ghwnd,ID_SHOW),temp);         
    return 1;
}
                                                                           
bool OnConnect()
{
     char ip[16];
     GetWindowText(GetDlgItem(ghwnd,ID_IP),ip,16);
     
     if(gConn.Connect(ip,LISTEN_PORT-1)==0){
             
             
             Report("�������ӵ�Զ�̵��ԣ�  Error Code ��%d  ",WSAGetLastError()); 
             
             return 0;
             }

     Paint("������");    // �˺�����VER 2 ��ִ���κβ���  
     FormatError("Success to connect remote macheine ");
	 
     if(gConn.GetPeerName(&gRemoteAddr)==0){
            FormatError("Can't Get Peer name ,Error Code : %d ",WSAGetLastError());
            return 0;
            }
     // Ϊ�����׽���ע�� ��Ϣ�Ա� ���ܵ���Ϣʱ ��ʱ��ʾ
     
     
     if(gConn.Send((char *)&gUdpPort,sizeof(int))==SOCKET_ERROR){
            FormatError("error when send the udp port ,Error code : %d ",WSAGetLastError());
            return 0;
            } 
     FormatError("Success to send udp port : %d ",gUdpPort);       
     if(gConn.Recv((char *)&gRemoteUdpPort,sizeof(int))==SOCKET_ERROR){
            FormatError("error when recv the udp port ,Error code : %d ",WSAGetLastError());
            return 0;
            }
     
     FormatError("success to recv the remote udp port : %d ",gRemoteUdpPort);
     if(gConn.AsyncSelect(ghwnd,MM_SOCKET_TEXT,FD_READ)==0){
            FormatError("error when AnsyncSelect the gConn,in OnConnect , Error Code : %d ",WSAGetLastError());
            return 0;
            }
     FormatError("Success to AsyncSelect the gConn in OnConnect");
     gThreadQuit=0; 
     gRecvThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )RecvThread,NULL,0,&gdwRecvPid);
     gSendThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )SendThread,NULL,0,&gdwSendPid);
     FormatError("Have already start the two thread ,in OnConnect() ");                      
     
     return 1;

}                                                                                   

int OnAccept()
{    
     //int  rtn ;
     SOCKET apt;
     apt=gListenSock.Accept();
	 if(apt==INVALID_SOCKET){
                         MessageBox(ghwnd,"����ʱ�������ܽ�����������"," ������һ������",MB_OK|MB_ICONERROR);
                         return 0;
                         } 
	 if(MessageBox(ghwnd,"�����������ӣ��Ƿ����","����",MB_OKCANCEL|MB_ICONINFORMATION)==IDCANCEL){
		      closesocket(apt);
			  return 0;
              
              }
     gConn=apt;
	 gThreadQuit=0;
	 // �������ͺͽ����߳� ������������
      
     gRecvThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )RecvThread,NULL,0,&gdwRecvPid);
     gSendThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )SendThread,NULL,0,&gdwSendPid);
     FormatError("Have already start the two thread send and recv thread,in OnAccept");  
     
     
     if(gConn.GetPeerName(&gRemoteAddr)==0){
            FormatError("Can't Get Peer name ,Error Code : %d ",WSAGetLastError());
            return 0;
            }
     // Ϊ�����׽��� ע�� �첽��Ϣ �Ա� ��ʱ��ȡ ������Ϣ
     if(gConn.Recv((char*)&gRemoteUdpPort,sizeof(int))==SOCKET_ERROR){
            FormatError("error when recv the remote udp port , Error Code : %d ",WSAGetLastError());
            return 0;
            }
     FormatError("Success to recv the remote udp port : %d ",gRemoteUdpPort);
     if(gConn.Send((char*)&gUdpPort,sizeof(int))==SOCKET_ERROR){
            FormatError("error when send the udp port , Error Code : %d ",WSAGetLastError());
            return 0;
            }
     FormatError("Success to send the udp port : %d ",gUdpPort);
     if(gConn.AsyncSelect(ghwnd,MM_SOCKET_TEXT,FD_READ)==0){
            FormatError("error when Asyncselect the gConn , in OnAccept Error Code : %d ", WSAGetLastError());
            return 0;
            }
     FormatError("Success to AsyncSelect the gConn");
     return 1;

} 



// ok this ProcessData  edited after multiThread

int ProcessData(WAVEHDR * hdr)
{
    //  ������������ܵ� MM_WIM_DATA �����
     
    //���潫�������ݱ��浽�������У��������̷߳���
     
    EnterCriticalSection(&gCs);
    //waveInUnprepareHeader(hdr);
    gWaveIn.UnPrepareHeader(hdr);
    memcpy(gData,hdr->lpData,hdr->dwBytesRecorded);
    gDataLen+=hdr->dwBytesRecorded;
    
    LeaveCriticalSection(&gCs);
    //FormatError("MM_WIM_DATA message have arrived ");
    
    //waveInPrepareHeader(hdr);
    gWaveIn.PrepareHeader(hdr);
    if(gWaveIn.AddBuffer(hdr)==0){
           FormatError("Failed to Add buffer ");
           return 0;
           }
    //FormatError("Success to Add buffer");
    //waveInAddBuffer(hWaveIn,hdr,sizeof(WAVEHDR));
    return 1;
   

}

// ��������ڽ����߳̽��ܵ��������ݺ���ã�����Ϊ�ܵ����ֽ�����    
int DataArrived(int len)
{
     //FormatError("Data Reved in  ");
     EnterCriticalSection(&gRecvCs);
     FormatError("���������ݱ��浽�طŻ���");
     
     // ��ȫ�ֻ���������Ӹո��ܵ������� 
     memcpy(gPlayBuff[gPlayBuffPtr],gRecv,len);
     
     //������д����Ƶ�豸 �Ա� ���� 
     if(gWaveOut.Write(gPlayBuff[gPlayBuffPtr],len)!=1){
           FormatError("Error when WaveOut.Write Buffer ,Error Code : %d ",GetLastError());
           //gPlayBuffPtr=(gPlayBuffPtr+1)%6;
           LeaveCriticalSection(&gRecvCs);
           return 0;
           }
     //ѭ������ȫ�ֻ����� 
     gPlayBuffPtr=(gPlayBuffPtr+1)%6;
      
     LeaveCriticalSection(&gRecvCs);
     return 1;


}                  

   
/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message){                  /* handle the messages */
    
        case WM_DESTROY:
             PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
             break;
        case WM_CREATE:
             Paint("Create");
             CreateWindowEx(0,"Button","Connect",WS_CHILD|WS_VISIBLE,20,20,100,25,hwnd,(HMENU)ID_CONNECT,ghInst,NULL);
             CreateWindowEx(0,"Button","DeConnect",WS_CHILD|WS_VISIBLE,20,50,100,25,hwnd,(HMENU)ID_DECONNECT,ghInst,NULL);
             CreateWindowEx(0,"Edit",""         ,WS_CHILD|WS_VISIBLE,130,20,200,25,hwnd,(HMENU)ID_IP   ,ghInst,NULL);
             CreateWindowEx(0,"Edit",""         ,WS_CHILD|WS_VISIBLE|ES_READONLY|ES_AUTOHSCROLL |ES_AUTOVSCROLL|ES_MULTILINE |WS_VSCROLL ,20,80,504,200,hwnd,(HMENU)ID_SHOW ,ghInst,NULL);
             CreateWindowEx(0,"Edit",""         ,WS_CHILD|WS_VISIBLE,20,285,504,60,hwnd,(HMENU)ID_INPUT,ghInst,NULL);
             CreateWindowEx(0,"Button","Send"       ,WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,20,355,100,20,hwnd,(HMENU)ID_SEND ,ghInst,NULL);
             
             if(gConn.Create()==0){
                    Paint("Failed to Create gConn");
                    return 0;
                    }
                    
             ghwnd=hwnd;
             Paint("gConn.Create");
             int nRet;
             // ��ʼ���׽��� 
             nRet=SockInit();
             if(nRet==0){
                    Paint("Failed to SockInit");
                    return 0;
                    }
             Paint("SockInit Success ");
             
             gCanRecv=CreateEvent(NULL,false ,false ,NULL);
             gCanSend=CreateEvent(NULL,false,false,NULL);
             gHaveData=CreateEvent(NULL,false,false,NULL);
             //ghRecv=CreateEvent(NULL,false,false,NULL);
             if(gCanRecv==NULL||gCanSend==NULL||gHaveData==NULL){
                     FormatError("Error when CreateEvent , Error Code : %d ",GetLastError());
                     Paint("Failed to Create the global event");
                     break;
                     }
                     
             
             Paint("Init the Global Event");
             FormatError("Init The Global Event");
             
             InitializeCriticalSection(&gCs);
             InitializeCriticalSection(&gRecvCs);
             
             break;
        case WM_PAINT:
			{
			 HDC hdc;
             PAINTSTRUCT ps;
             hdc=BeginPaint(hwnd,&ps);
             for(int i=0;i<nPaintNum;i++){
                     
             TextOut(hdc,20,80+i*20,cPaint[i],strlen(cPaint[i]));
             }
             EndPaint(hwnd,&ps);
			}
             break;
             
             
        case WM_COMMAND:
             if( (LOWORD(wParam))==ID_CONNECT ){
                       if(OnConnect()){
                                 //������ӳɹ�������˷� 
                                  if(gWaveIn.Init(hwnd)!=1 ){
                                           //MessageBox(NULL,"Failed to Open WaveIn","Error",MB_OK|MB_ICONINFORMATION);
                                           Report("Failed to open waveIn ,Error code : %d ",GetLastError());
                                           
                                           return 0;
                                           }
                                         
                                  //if(waveInStart(hWaveIn)!=MMSYSERR_NOERROR){
                                  if(gWaveIn.Start()!=1){
                                           #ifdef debug
                                           //MessageBox(NULL,"Failed to Start hWaveIn","Error",MB_OK);
                                           //Report("Failed to Start hWaveIn, Error code : %d ",GetLastError());
                                           Report("Failed to Start hWaveIn, Error code : %d ",(int)gWaveIn.GetRet());
                                           return 0;
                                           #endif
                                           }
                                  Paint("Success to Start WaveIn ");
                                  AddShowText("...������ ��˷� �� �����������..."); 

                                  bRecording=true;
                                  
                                  //���������豸 
                                  if(gWaveOut.Init(hwnd)!=1){
                                           Report("Failed to open waveOut ,Error code : %d ",GetLastError());
                                           return 0;
                                           }
                                  AddShowText("...������ ��Ƶ�豸��׼����������..."); 
                                  
                                  }

             }
             if((LOWORD(wParam))==ID_SEND){
                       FormatError("Call to OnSendText");                    
                       OnSendText();
                       //����������Ϣ 
                       }
			 if((LOWORD(wParam))==ID_DECONNECT){
				       gConn.Close();
					   FormatError("Close the connection");
					   gThreadQuit=1;
					   //ֹͣ�����߳� 
			 }
                                        
             break;
        case WM_QUIT:
             if(bRecording){
                            bRecording=false;
                            //waveInClose(hWaveIn);
                            gWaveIn.Close();
                            }
             if(bPlaying){
                          bPlaying=false;
                          //waveOutReset(hWaveOut);
                          //waveOutClose(hWaveOut);
                          gWaveOut.Reset();
                          gWaveOut.Close();
                          }
             
             
             /*
             if(pSaveBuffer!=NULL){
                                 free(pSaveBuffer);
                                 }
             
             */
             //SockClean();
             
             DeleteCriticalSection(&gCs);
             DeleteCriticalSection(&gRecvCs);
             CloseHandle(gHaveData);
             CloseHandle(gCanRecv);
             CloseHandle(gCanSend);
             //CloseHandle(ghRecv);
             
             CloseHandle(gRecvThread);
             CloseHandle(gSendThread);
			 if(!gThreadQuit){
				    gThreadQuit=1;
			 }
			 DestroyWindow(hwnd);
             break;
        case MM_SOCKET_TEXT:
             if(WSAGETSELECTERROR(lParam)){
                    gConn.Close();
                    FormatError("error when mm_socket_text");
                    return 0;
                    }else{
                          
             switch(WSAGETSELECTEVENT(lParam)){
                    case FD_READ:
                         FormatError("call to OnRecvText()");
                         OnRecvText();
                         break;
                    }
             
             }
             break;
        case MM_SOCKETLISTEN:
             if(WSAGETSELECTERROR(lParam)){
                    gListenSock.Close();
                    Paint("error when mm_socketlisten ");
                    break;
                    }
             switch(WSAGETSELECTEVENT(lParam)){
                    case FD_ACCEPT:
                         FormatError("Recved the request to connect");
                         Paint      ("Recved the request to connect");
                         if(OnAccept()){
                                  if(gWaveIn.Init(hwnd)!=1 ){
                                           //MessageBox(NULL,"Failed to Open WaveIn","Error",MB_OK|MB_ICONINFORMATION);
                                           Report("Failed to open waveIn ,Error code : %d ",GetLastError());
                                           
                                           return 0;
                                           }
                                         
                                  //if(waveInStart(hWaveIn)!=MMSYSERR_NOERROR){
                                  if(gWaveIn.Start()!=1){
                                           #ifdef debug
                                           //MessageBox(NULL,"Failed to Start hWaveIn","Error",MB_OK);
                                           //Report("Failed to Start hWaveIn, Error code : %d ",GetLastError());
                                           Report("Failed to Start hWaveIn, Error code : %d ",(int)gWaveIn.GetRet());
                                           return 0;
                                           #endif
                                           }
                                  Paint("Success to Start WaveIn ");

                                  bRecording=true;
                                  AddShowText("...������ ��˷磬 �����ʹ������...");
                                   
                                  if(gWaveOut.Init(ghwnd ) !=1){
                                           Report("Failed to open waveOut,Error Code : %d",GetLastError());
                                           return 0;
                                           } 
                                  AddShowText("...��������Ƶ�豸�������ʹ������...");  
                                  }         
                                             
                                          
                         //gRecvThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )RecvThread,NULL,0,&gdwRecvPid);
                         //gSendThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE )SendThread,NULL,0,&gdwSendPid);
                         //Paint("Create Thread RecvThread , SendThread ");
                         
                         break;
                         }
                    
        case MM_SOCKETDATA_CLIENT:                         //  edited after multlThread
             if(WSAGETSELECTERROR(lParam)){
	                FormatError("MM_SOCKETDATA arrived ,but some error occured ,Error code : %d ",WSAGetLastError());
                    gClient.Close();
                    break;
             }else{
             switch(WSAGETSELECTEVENT(lParam)){
                    //case FD_READ:
  	    	        //     FormatError("FD_READ event arrived ");
                    //     SetEvent(ghRecv);
                    //     break;
                    case FD_WRITE:
                         //FormatError("FD_WRITE event arrived");
                         FormatError("Ok ֪ͨ�����̷߳�������"); 
                         // ֪ͨ�����߳̿��Է��������� 
                         SetEvent(gCanSend);
                         break;
                    }
                         
             }           
             
             break;
        
        case MM_SOCKETDATA_RECV:
             if(WSAGETSELECTERROR(lParam)){
                     FormatError("MM_SOCKETDATA_RECV arrived ,but some error occured");
                     gRecvSock.Close();
                     }
             else{
             switch(WSAGETSELECTEVENT(lParam)){
                     case FD_READ:
                          FormatError("OK ֪ͨ�����߳̽�������");
                          SetEvent(gCanRecv);
                          // ֪ͨ�����̣߳����������ݵ��� 
                          break;
                          }
             }
             break;
        
        case MM_WIM_OPEN:
             
                
                 break;
        case MM_WIM_DATA:
                 // ��˷���¼������һ������ 
                 
		         ProcessData((WAVEHDR *)lParam);
		         
		         // ֪ͨ�����߳���������Ҫ����
                  
                 SetEvent(gHaveData);     
                 break;
        case MM_WIM_CLOSE:
                 //WaveInClear();
                 // Clean ������ڲ�����Ļ��� 
                 gWaveIn.Clean();
                 break;
        case MM_WOM_DONE:
                 break;
                 
             
            
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    
    }
    return 0;
}

int SockInit()
{   
    SOCKET listensock;
    //sockaddr_in inetaddr;
    listensock=socket(AF_INET,SOCK_STREAM,0);
    if(listensock==INVALID_SOCKET){
                                   
                                   Paint("create listensock failed ");
                                   return 0;
                                   }
                                   
    gListenSock=listensock;
    
    /* the old version before my writting class Socket
    
    if(WSAAsyncSelect(listensock,ghwnd,MM_SOCKETLISTEN,FD_ACCEPT|FD_CLOSE)==SOCKET_ERROR){
                Report("Error when async the listen socket ,Error code : %d ",WSAGetLastError());
                return 0;
                }
    Paint("Success to async the listen socket ");
    inetaddr.sin_addr.s_addr=INADDR_ANY;
    inetaddr.sin_family     =AF_INET;
    inetaddr.sin_port       =htons(MY_PORT);
    
    if((listensock,(sockaddr*)&inetaddr,sizeof(inetaddr))==SOCKET_ERROR){
                FormatError("Error when bind ,Error code: %d ,at line: %d ;",WSAGetLastError(),__LINE__);
                
                Paint("Error when bind");
                closesocket(listensock);
                WSACleanup( );
                return 0;
                }
    FormatError("Sucess to bind to local port");
    Paint("bind");
    
    */
    
    if(gListenSock.Bind(LISTEN_PORT)==0){
                FormatError("Error when Bind the gListenSock,Error Code : %d ",WSAGetLastError());
                Paint("Error when Bind");
                return 0;
		  }
    if(gListenSock.AsyncSelect(ghwnd,MM_SOCKETLISTEN,FD_ACCEPT|FD_CLOSE)==0){
	         FormatError("Error to ansyncselect the gListenSock ,Error Code : %d ",WSAGetLastError());
	         Paint("Error when AnsyncSelect");
		  return 0;
		  
		  }
    FormatError("Success to ansyncselect the gListenSock,code : %d " ,WSAGetLastError());
		  
    
    //if(listen(listensock,5)==SOCKET_ERROR){
    if(gListenSock.Listen()==0){
                FormatError("Error when listen,Error code: %d, at line: %d ;",WSAGetLastError(),__LINE__);
                closesocket(listensock);
                WSACleanup( );
                return 0;
                }
    FormatError("Listening");
    Paint("Listening ");
    gListenSock=listensock;
                
    /* �ɰ汾 �� 
    SOCKET client=socket(AF_INET,SOCK_DGRAM,0);
    
    if(client==INVALID_SOCKET){
                closesocket(client);
                closesocket(listensock);
                WSACleanup( );
                return 0;
                }
    gClient=client; 
    */
    if(gClient.Create(SOCK_DGRAM)==0){
                FormatError(" Error when create the gClient socket ,Error Code : %d ",WSAGetLastError());
                return 0;
                }
    
    if(gRecvSock.Create(SOCK_DGRAM)==0){
                FormatError(" Error when create the gRedvsock socket ,Error Code : %d ",WSAGetLastError());
                return 0;
                }
    
    
    //WSAAsyncSelect(client,ghwnd,MM_SOCKETDATA,FD_READ|FD_WRITE);
    if(gClient.AsyncSelect(ghwnd,MM_SOCKETDATA_CLIENT,FD_WRITE)==0){
                FormatError("error when AsyncSelect the gClient ,Error Code : %d ",WSAGetLastError());
		        return 0;
		        }
    FormatError("Success to Ansyncselect the gClient ,code : %d ",WSAGetLastError());
    
    /* �ɰ汾 
    if(gClient.Bind(UDP_PORT)==0){
                 FormatError("error when bind the gClient,Error Code : %d ",WSAGetLastError());
                 return 0;
                 }
                 
    */
    /*
    if(gClient.AsyncSelect(ghwnd,MM_SOCKERT_TEXT,FD_READ)==0){
                 FormatError("error when AsyncSelect the gClient,Error Code : %d ",WSAGetLastError());
                 return 0;
                 }
    */
    
    if(gRecvSock.Bind(UDP_PORT)==0){
                 int   i=10;
                 while(i--){
                          gUdpPort+=i;
                          if(gRecvSock.Bind(gUdpPort)==SOCKET_ERROR){
                                 
                                 if(i<1){
                                         return 0;
                                         }
                                 continue;
                                 }
                          else
                                  break;
                 //FormatError("error when bind the gRecvSock ,Error Code : %d ",WSAGetLastError());
                 //return 0;
                 }
                 }
                 
   
    
    if(gRecvSock.AsyncSelect(ghwnd,MM_SOCKETDATA_RECV,FD_READ)==0){
                 FormatError("error when AsyncSelect the gRecvSock,Error Code : %d ",WSAGetLastError());
                 return 0;
                 }
    
    return 1;
}

DWORD  WINAPI SendThread(LPVOID lParam)
{	
	int nSend;
	
	while(!gThreadQuit){

	        //FormatError("Begin to Wait MM_WIM_DATA ,in SendThread , in Client");
	        // �ȴ� �������ݵ���  
	        FormatError("[SendThread] �ȴ��������ݵ���");
            if(WaitForSingleObject(gHaveData,INFINITE)== WAIT_FAILED){
	     		    FormatError("Error when WaitFor gHaveData ,Error Code : %d",GetLastError()) ;
			        continue;
			}
            
            //FormatError("Ok , ready to send data ,in SendThread");
	        //FormatError("Begin to Wait FD_SEND event ,in Client ");
	     	// �ȴ� ���Է������� ���¼� 
            if(WaitForSingleObject(gCanSend, INFINITE)== WAIT_FAILED ){
			        //FormatError("Error when WaitFor gCanSend ,Error Code %d",GetLastError())   ;
			        continue;
			}
            //FormatError("OK ,then I'll call to sendto ");
            
            char ip[17];
            // ��ѯ�����ӵ�Զ�� IP  
            if(gConn.GetPeerName(ip)==0){
                                         
                    //FormatError("[ SendThread ] error when GetPeerName ,Error Code : %d ",WSAGetLastError());
                    break;
                    }
			
                     nSend=gClient.Sendto(gData,gDataLen,ip,gRemoteUdpPort);
			         if(nSend==SOCKET_ERROR){

				            if(WSAGetLastError()==WSAEWOULDBLOCK){
	   			                    //FormatError("The data Sent wouldblock ,I'll try later");
					                SetEvent(gHaveData);
					                continue;
                                    }
				                    else{
				
				                    FormatError("SendError in SendThread : %d",WSAGetLastError());
				
				
				
				                    break;
                		            }
                             }else{
			

				             EnterCriticalSection(&gCs);
				             gDataLen=gDataLen-nSend;
				             if(gDataLen<0){
                                            gDataLen=0;
                                            }
                             if(nSend>0){
                                         //FormatError("Call to memcpy ");
				                         memcpy((void*)gData,(void*)&gData[nSend],gDataLen);
                                         }
				             if(gDataLen>0){
					                  SetEvent(gHaveData);
					                  // һ��û�з��������ݣ��������� 
					                  }
                             LeaveCriticalSection(&gCs);
                             if(nSend>0){
                                         
                                       FormatError("Send %d bytes data ,to : %s:%d",nSend,ip,gRemoteUdpPort);
                                       SetEvent(gCanSend);
                                      
                                       }
			                 }
	                 
                 }  //  while 
	FormatError(" SendThread terminited ");
    return 0 ;
}


DWORD  WINAPI RecvThread(LPVOID lParam)
{
	int nRecv=0;
	int addrlen=sizeof(sockaddr_in);
	sockaddr_in inetaddr;
	while(!gThreadQuit){
	
	//WaitForSingleObject(ghRecv,INFINITE);
	FormatError("[RecvThread] �ȴ��������ݵ���");
	//�ȴ��������ݵ��� 
    WaitForSingleObject(gCanRecv,INFINITE);
    //FormatError("Enter the critical section gRecvCS");
	EnterCriticalSection(&gRecvCs);
	//FormatError("then I'll call to recvfrom");
	nRecv=gRecvSock.Recvfrom(gRecv,RECVBUF_SIZE,&inetaddr,&addrlen);
		
		
        if(nRecv==SOCKET_ERROR){
			if(WSAGetLastError()==WSAEWOULDBLOCK){
				SetEvent(gCanRecv);
				LeaveCriticalSection(&gRecvCs);
				continue;
			}
			else{
			
            	
			FormatError("recvError in RecvThread : %d\n",WSAGetLastError());
			//clean();
			LeaveCriticalSection(&gRecvCs);
			
			break;
			}
	     
         }
     LeaveCriticalSection(&gRecvCs);
     if(nRecv>0){
                 FormatError("Recved %d bytes ",nRecv);
     
                 DataArrived(nRecv);
                 }
     }
     FormatError("RecvThread termited ");
	 return 1;
}                 
                
