VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CEvent"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Public Enum tagEventId
    EIUnknown = 0
    EICreateTask
    EIActivateTask
    EIDeactivateTask
    EICloseTask
    EIStart
    EIPause
End Enum

Public EventID As tagEventId
Public TimeStamp As New CDatum
Public Task As CTask

Private Sub Persist()
    Dim f As Long

    f = FreeFile
    Open csEventFile For Append Access Write Lock Write As #f
    Print #f, EventID; "^"; TimeStamp.asDouble; "^"; Task.ID; "^"; Task.Text
    Close #f
End Sub

Public Sub Initialize(ByVal aTask As CTask, ByVal anEvent As tagEventId)
    Set Task = aTask
    EventID = anEvent
End Sub

Private Sub Class_Initialize()
    TimeStamp.SetNow
End Sub

Private Sub Class_Terminate()
    If Not Task Is Nothing Then
        If Task.State <> EIUnknown Then Persist
    End If
End Sub
