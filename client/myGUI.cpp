#include <wx/wx.h>
#include "client.h"  // Giả sử đây là class Client mà ta sẽ định nghĩa

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    wxTextCtrl* emailTextCtrl;
    wxTextCtrl* displayTextCtrl;
    Client client;

    void OnConnect(wxCommandEvent& event);
    void OnListApp(wxCommandEvent& event);
    void OnListServices(wxCommandEvent& event);
    void OnScreenshot(wxCommandEvent& event);
    void OnShutdown(wxCommandEvent& event);
    void OnStartApp(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_CONNECT = 1,
    ID_LIST_APP,
    ID_LIST_SERVICES,
    ID_SCREENSHOT,
    ID_SHUTDOWN,
    ID_START_APP
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_BUTTON(ID_CONNECT, MyFrame::OnConnect)
EVT_BUTTON(ID_LIST_APP, MyFrame::OnListApp)
EVT_BUTTON(ID_LIST_SERVICES, MyFrame::OnListServices)
EVT_BUTTON(ID_SCREENSHOT, MyFrame::OnScreenshot)
EVT_BUTTON(ID_SHUTDOWN, MyFrame::OnShutdown)
EVT_BUTTON(ID_START_APP, MyFrame::OnStartApp)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("Client-Server Application");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400)) {

    wxPanel* panel = new wxPanel(this, -1);

    new wxStaticText(panel, wxID_ANY, "IP Address (Gmail):", wxPoint(10, 10));
    emailTextCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(150, 10), wxSize(200, -1));

    new wxButton(panel, ID_CONNECT, "Connect", wxPoint(370, 10));
    new wxButton(panel, ID_LIST_APP, "List App", wxPoint(10, 50));
    new wxButton(panel, ID_LIST_SERVICES, "List Services", wxPoint(100, 50));
    new wxButton(panel, ID_SCREENSHOT, "Screenshot", wxPoint(200, 50));
    new wxButton(panel, ID_SHUTDOWN, "Shutdown", wxPoint(310, 50));
    new wxButton(panel, ID_START_APP, "Start App", wxPoint(410, 50));

    displayTextCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(10, 100), wxSize(550, 250), wxTE_MULTILINE | wxTE_READONLY);
}

void MyFrame::OnConnect(wxCommandEvent& event) {
    std::string serverIP = emailTextCtrl->GetValue().ToStdString();
    client.setServerIP(serverIP);
    if (client.connectToServer()) {
        displayTextCtrl->AppendText("Connected to server\n");
    }
    else {
        displayTextCtrl->AppendText("Failed to connect to server\n");
    }
}

void MyFrame::OnListApp(wxCommandEvent& event) {
    std::string response = client.sendMessage("1");
    displayTextCtrl->AppendText("List App:\n" + response + "\n");
}

void MyFrame::OnListServices(wxCommandEvent& event) {
    std::string response = client.sendMessage("2");
    displayTextCtrl->AppendText("List Services:\n" + response + "\n");
}

void MyFrame::OnScreenshot(wxCommandEvent& event) {
    std::string response = client.sendMessage("3");
    displayTextCtrl->AppendText("Screenshot:\n" + response + "\n");
}

void MyFrame::OnShutdown(wxCommandEvent& event) {
    std::string response = client.sendMessage("4");
    displayTextCtrl->AppendText("Shutdown:\n" + response + "\n");
}

void MyFrame::OnStartApp(wxCommandEvent& event) {
    // Hiển thị hộp thoại nhập đường dẫn ứng dụng
    wxTextEntryDialog dialog(this, "Enter the full path of the application to start:", "Start Application");

    if (dialog.ShowModal() == wxID_OK) {
        std::string appPath = dialog.GetValue().ToStdString();  // Lấy đường dẫn từ hộp thoại

        // Gửi yêu cầu đến server để khởi động ứng dụng
        std::string response = client.sendMessage("on_app " + appPath);
        displayTextCtrl->AppendText("Start App:\n" + response + "\n");
    }
}
