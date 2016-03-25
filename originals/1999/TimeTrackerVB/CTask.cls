VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CTask"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit
Option Base 1

Public ID As String
Public Text As String
Public Created As New CDatum
Public Closed As New CDatum
Public LastPaused As New CDatum
Public LastStart As New CDatum
Private theWorkedToday As Date
Private theState As tagTaskState
Private theRunning As Boolean

Private theChunks As Long
Private theRepStart As New CDatum
Private theTotalTime As Date

Public Enum tagSubItemIndex
    SIICreated = 1
    SIIClosed = 2
    SIILastPause = 3
    SIIWorked = 4
End Enum

Public Enum tagTaskState
    TSUnknown
    TSDeactivate
    TSActive
    TSClosed
End Enum

Public Property Get WorkedToday() As Date
    Dim n As New CDatum
    If Running Then
        n.SetNow
        WorkedToday = theWorkedToday + LastStart.TimeTo(n)
    Else
        WorkedToday = theWorkedToday
    End If
End Property

Public Property Get Running() As Boolean
    Running = theRunning
End Property

Public Property Let Running(ByVal rhs As Boolean)
    Dim e As New CEvent
    
    If rhs <> theRunning Then
        theRunning = rhs
        If theRunning Then
            LastStart.SetNow
            e.Initialize Me, EIStart
        Else
            LastPaused.SetNow
            e.Initialize Me, EIPause
            theWorkedToday = theWorkedToday + LastStart.TimeTo(LastPaused)
        End If
    End If
End Property


Public Property Get State() As tagTaskState
    State = theState
End Property

Public Property Let State(ByVal rhs As tagTaskState)
    Dim e As New CEvent
    
    If theState <> rhs Then
        If theState = TSUnknown Then
            theState = rhs
        Else
            theState = rhs
            Select Case theState
                Case TSDeactivate
                    Running = False
                    e.Initialize Me, EIDeactivateTask
                    Closed.SetNull
                Case TSActive
                    e.Initialize Me, EIActivateTask
                    Closed.SetNull
                    Running = True
                Case TSClosed
                    Running = False
                    Closed.SetNow
                    e.Initialize Me, EICloseTask
            End Select
        End If
    End If
End Property

Public Sub FromLV(ByVal LV As ListView)
    FromLI LV.ListItems(ID)
End Sub

Public Sub ToLV(ByVal LV As ListView)
    ToLI LV.ListItems(ID)
End Sub

Public Sub ToLI(ByVal li As ListItem)
    With li
        Set .Tag = Me
        .Text = Text
        .SubItems(SIICreated) = Created.asString
        .SubItems(SIIClosed) = Closed.asString
        If theTotalTime > 0 Then
            .SubItems(SIIWorked) = _
                FormatDateTime(WorkedToday, vbShortTime) & " (" & _
                FormatDateTime(theTotalTime, vbShortTime) & ")"
        Else
            .SubItems(SIIWorked) = FormatDateTime(WorkedToday, vbShortTime)
        End If
        .Ghosted = False
        If Running Then
            .SubItems(SIILastPause) = LastStart.asString
            .SmallIcon = "RUNNING"
        Else
            .SubItems(SIILastPause) = LastPaused.asString
            If State = TSActive Then
                .SmallIcon = "ACTIVE"
            ElseIf State = TSDeactivate Then
                .SmallIcon = "INACTIVE"
            ElseIf State = TSClosed Then
                .SmallIcon = "CLOSED"
                .Ghosted = True
            Else
                .SmallIcon = "UNKNOWN"
            End If
        End If
    End With
End Sub

Public Sub FromLI(ByVal li As ListItem)
    Text = li.Text
End Sub

Private Sub Class_Initialize()
    ID = "#" & CStr(CLng(Rnd() * &H7FFFFFF0))
    theState = EIUnknown
    theRunning = False
    theWorkedToday = 0
    Text = "New Task " & ID
    Created.SetNow
End Sub

Private Sub Class_Terminate()
    Running = False
    Persist
End Sub

Public Function This() As CTask
    Set This = Me
End Function

Private Sub Persist()
    Dim f As Integer
    
    f = frmMain.GetTaskFile
    Print #f, ID; "^"; State; "^"; Created.asDouble; "^"; Closed.asDouble; "^"; LastPaused.asDouble; "^"; Text
End Sub

Public Sub fromString(ByVal s As String)
    Dim v As Variant
    
    v = Split(s, "^")
    ID = v(0)
    theState = v(1)
    Created.fromDouble v(2)
    Closed.fromDouble v(3)
    LastPaused.fromDouble v(4)
    Text = v(5)
End Sub

'----------

Public Sub PrepareReport()
    theTotalTime = CDate(0)
    theChunks = 0
    theRepStart.SetNull
End Sub

Public Sub Report(ByVal R As CReport)
    If R.EventID = EIStart Then
        Debug.Assert theRepStart.isNull
        theRepStart.asDate = R.TS.asDate
        theChunks = theChunks + 1
    ElseIf R.EventID = EIPause Then
        theTotalTime = theTotalTime + theRepStart.TimeTo(R.TS)
        theRepStart.SetNull
    End If
End Sub

Public Function ReportResult() As String
    Dim d As New CDatum
    If theChunks > 0 Then
        If Not theRepStart.isNull Then
            d.SetNow
            theTotalTime = theTotalTime + theRepStart.TimeTo(d)
        End If
        ReportResult = Format$(theTotalTime, "Hh:Nn") & _
            " in " & theChunks & "x " & Text & vbCrLf
    End If
End Function
