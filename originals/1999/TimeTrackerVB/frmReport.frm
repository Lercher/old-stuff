VERSION 5.00
Begin VB.Form frmReport 
   Caption         =   "Report"
   ClientHeight    =   4095
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6615
   Icon            =   "frmReport.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   4095
   ScaleWidth      =   6615
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox theText 
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   3075
      Left            =   0
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   0
      Top             =   0
      Width           =   6075
   End
End
Attribute VB_Name = "frmReport"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Form_Resize()
    On Error Resume Next
    theText.Move 0, 0, ScaleWidth, ScaleHeight
End Sub

Public Sub Report()
    Dim s As String
    Dim dReport As New CDatum
    
    s = InputBox("Reportdate", "Please Enter", FormatDateTime(Now, vbShortDate))
    If Len(s) Then
        dReport.asDate = s
        Show
        ReadEvents dReport
    End If
End Sub

Private Sub ReadEvents(ByVal dReport As CDatum)
    Dim f As Integer
    Dim AllTasks As Collection
    Dim s As String
    Dim T As CTask
    Dim R As CReport
    Dim serr As String
    
    Caption = "Report " & FormatDateTime(dReport.asDate, vbShortDate)
    Set AllTasks = frmMain.AllTasks
    
    For Each T In AllTasks
        T.PrepareReport
    Next
    
    Set R = New CReport
    f = FreeFile
    Open csEventFile For Input Access Read As #f
    Do While Not EOF(f)
        Line Input #f, s
        s = Trim$(s)
        R.fromString s
        If R.Filter(dReport) Then
            Set T = Nothing
            On Error Resume Next
            Set T = AllTasks(R.TaskID)
            On Error GoTo 0
            If T Is Nothing Then
                s = R.asString
                If Len(s) Then serr = serr & s & vbCrLf
            Else
                T.Report R
            End If
        End If
    Loop
    Close #f
    
    s = serr
    For Each T In AllTasks
        s = s & T.ReportResult
        T.ToLV frmMain.getListview
    Next
    
    theText = s
End Sub
