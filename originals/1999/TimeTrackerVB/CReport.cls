VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CReport"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'Print #f, EventID; "^"; TimeStamp.asDouble; "^"; Task.ID; "^"; Task.Text

Public EventID As tagEventId
Public TS As New CDatum
Public TaskID As String
Public taskName As String

Public Sub fromString(ByVal s As String)
    Dim v As Variant
    
    v = Split(s, "^")
    EventID = v(0)
    TS.fromDouble v(1)
    TaskID = v(2)
    taskName = v(3)
End Sub

Public Function Filter(ByVal d As CDatum) As Boolean
    Filter = d.isEqual(TS)
End Function

Public Function asString() As String
    Dim ev As String
    If EventID = EIStart Then
        ev = "Start"
    ElseIf EventID = EIPause Then
        ev = "Stop"
    End If
    If Len(ev) Then
        asString = TS.asString & " " & ev & " " & taskName
    End If
End Function

