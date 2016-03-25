VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmMain 
   Caption         =   "TimeTrak"
   ClientHeight    =   4230
   ClientLeft      =   165
   ClientTop       =   735
   ClientWidth     =   8745
   Icon            =   "frmMain.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   4230
   ScaleWidth      =   8745
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrRefresh 
      Interval        =   10000
      Left            =   720
      Top             =   2400
   End
   Begin MSComctlLib.ImageList ilSmall 
      Left            =   60
      Top             =   2400
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   16
      MaskColor       =   12632256
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   5
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":014A
            Key             =   "CLOSED"
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":02B2
            Key             =   "INACTIVE"
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":041A
            Key             =   "UNKNOWN"
         EndProperty
         BeginProperty ListImage4 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":0582
            Key             =   "ACTIVE"
         EndProperty
         BeginProperty ListImage5 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":06EA
            Key             =   "RUNNING"
         EndProperty
      EndProperty
   End
   Begin MSComctlLib.ListView lvTasks 
      Height          =   2355
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   4755
      _ExtentX        =   8387
      _ExtentY        =   4154
      View            =   3
      Arrange         =   2
      LabelWrap       =   -1  'True
      HideSelection   =   -1  'True
      AllowReorder    =   -1  'True
      FullRowSelect   =   -1  'True
      GridLines       =   -1  'True
      _Version        =   393217
      SmallIcons      =   "ilSmall"
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      BorderStyle     =   1
      Appearance      =   1
      NumItems        =   5
      BeginProperty ColumnHeader(1) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         Text            =   "Title"
         Object.Width           =   4939
      EndProperty
      BeginProperty ColumnHeader(2) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   1
         Text            =   "Created"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(3) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   2
         Text            =   "Closed"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(4) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   3
         Text            =   "Last Event"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(5) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   4
         Text            =   "Time"
         Object.Width           =   2540
      EndProperty
   End
   Begin VB.Menu mnuTask 
      Caption         =   "&Task"
      Begin VB.Menu mnuNew 
         Caption         =   "&New ..."
         Shortcut        =   ^N
      End
      Begin VB.Menu mnuCloseTask 
         Caption         =   "&Close"
         Shortcut        =   ^C
      End
      Begin VB.Menu mnuOpen 
         Caption         =   "&Open"
         Shortcut        =   ^O
      End
      Begin VB.Menu mnuActivate 
         Caption         =   "&Activate"
         Shortcut        =   ^A
      End
      Begin VB.Menu mnuReport 
         Caption         =   "&Report"
         Shortcut        =   ^R
      End
      Begin VB.Menu mnuQuit 
         Caption         =   "&Quit"
         Shortcut        =   ^Q
      End
   End
   Begin VB.Menu mnuActive 
      Caption         =   "&Active Task"
      Begin VB.Menu mnuStart 
         Caption         =   "&Start"
         Shortcut        =   ^S
      End
      Begin VB.Menu mnuPause 
         Caption         =   "&Pause"
         Shortcut        =   ^P
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private ActiveTask As CTask
Private theTaskFile As Integer

Private Sub Form_Initialize()
    Randomize
End Sub

Private Sub Form_Load()
    Dim f As Integer
    Dim s As String
    Dim li As ListItem
    Dim T As CTask
    Dim c As Long
    
    On Error GoTo out
    f = FreeFile
    Open csTaskFile For Input Access Read As #f
    On Error GoTo 0
    c = 0
    Do While Not EOF(f)
        Line Input #f, s
        s = Trim$(s)
        If Len(s) Then
            Set T = New CTask
            T.fromString s
            If T.State = TSClosed Then
                Set li = lvTasks.ListItems.Add(, T.ID)
            Else
                c = c + 1
                Set li = lvTasks.ListItems.Add(c, T.ID)
            End If
            T.ToLI li
            If T.State = TSActive Then
                Set ActiveTask = T
                Set lvTasks.SelectedItem = li
            End If
        End If
    Loop
    Close #f
out:
    DoUI
End Sub

Private Sub Form_Resize()
    On Error Resume Next
    lvTasks.Move 0, 0, ScaleWidth, ScaleHeight
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Set ActiveTask = Nothing
    lvTasks.ListItems.Clear
    If theTaskFile > 0 Then Close #theTaskFile
End Sub

Private Sub lvTasks_AfterLabelEdit(Cancel As Integer, NewString As String)
    On Error Resume Next
    CurrentTask.Text = NewString
    DoUI
End Sub

Private Sub lvTasks_DblClick()
    On Error Resume Next
    With CurrentTask
        If .Running Then
            .Running = False
            .ToLV lvTasks
        Else
            mnuActivate_Click
        End If
    End With
    DoUI
End Sub

Private Sub lvTasks_KeyDown(KeyCode As Integer, Shift As Integer)
    If KeyCode = vbKeyF2 Then lvTasks.StartLabelEdit
    If KeyCode = vbKeyReturn Then lvTasks_DblClick
    If KeyCode = vbKeyInsert Then mnuNew_Click
    If KeyCode = vbKeyDelete Then mnuCloseTask_Click
End Sub

Private Sub mnuActivate_Click()
    On Error Resume Next
    If Not ActiveTask Is Nothing Then
        ActiveTask.State = TSDeactivate
        ActiveTask.ToLV lvTasks
    End If
    With CurrentTask
        .State = TSActive
        .ToLV lvTasks
        Set ActiveTask = .This
    End With
    DoUI
End Sub

Private Sub mnuCloseTask_Click()
    On Error Resume Next
    With CurrentTask
        .State = TSClosed
        .ToLV lvTasks
    End With
End Sub

Private Sub mnuNew_Click()
    Dim li As ListItem
    Dim T As New CTask
    
    On Error Resume Next
    T.State = TSDeactivate
    Set li = lvTasks.ListItems.Add(1, T.ID)
    T.ToLI li
    Set lvTasks.SelectedItem = li
    lvTasks.Refresh
    lvTasks.StartLabelEdit
End Sub

Private Sub mnuOpen_Click()
    On Error Resume Next
    With CurrentTask
        .State = TSDeactivate
        .ToLV lvTasks
    End With
End Sub

Private Sub mnuQuit_Click()
    Unload Me
End Sub

Private Sub mnuPause_Click()
    On Error Resume Next
    With ActiveTask
        .Running = False
        .ToLV lvTasks
    End With
    DoUI
End Sub

Private Sub mnuReport_Click()
    Dim f As New frmReport
    f.Report
End Sub

Private Sub mnuStart_Click()
    On Error Resume Next
    With ActiveTask
        .Running = True
        .ToLV lvTasks
    End With
    DoUI
End Sub

Private Function CurrentTask() As CTask
    On Error Resume Next
    Set CurrentTask = lvTasks.SelectedItem.Tag
End Function

Private Sub tmrRefresh_Timer()
    DoUI
End Sub

Public Function GetTaskFile() As Integer
    If theTaskFile = 0 Then
        theTaskFile = FreeFile
        Open csTaskFile For Output Access Write Lock Write As #theTaskFile
    End If
    GetTaskFile = theTaskFile
End Function

Private Sub DoUI()
    If Not ActiveTask Is Nothing Then
        ActiveTask.ToLV lvTasks
        Caption = ActiveTask.Text & " - TimeTrak"
        Set Icon = ilSmall.ListImages(lvTasks.ListItems(ActiveTask.ID).SmallIcon).Picture
    End If
End Sub

Public Function AllTasks() As Collection
    Dim li As ListItem
    Dim T As CTask
    
    Set AllTasks = New Collection
    For Each li In lvTasks.ListItems
        Set T = li.Tag
        AllTasks.Add T, T.ID
    Next
End Function

Public Function getListview() As ListView
    Set getListview = lvTasks
End Function
