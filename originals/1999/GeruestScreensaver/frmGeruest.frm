VERSION 5.00
Begin VB.Form frmMain 
   BackColor       =   &H80000005&
   Caption         =   "Minimal Distance Connected Graph"
   ClientHeight    =   5760
   ClientLeft      =   165
   ClientTop       =   450
   ClientWidth     =   8715
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   ScaleHeight     =   5760
   ScaleWidth      =   8715
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer Timer2 
      Enabled         =   0   'False
      Interval        =   2500
      Left            =   1380
      Top             =   4380
   End
   Begin VB.Frame Points 
      BorderStyle     =   0  'None
      Caption         =   "Frame1"
      Height          =   195
      Index           =   0
      Left            =   2400
      TabIndex        =   1
      Top             =   2280
      Visible         =   0   'False
      Width           =   195
   End
   Begin VB.Timer Timer1 
      Enabled         =   0   'False
      Interval        =   500
      Left            =   660
      Top             =   4320
   End
   Begin VB.OptionButton Marker 
      Appearance      =   0  'Flat
      BackColor       =   &H80000005&
      ForeColor       =   &H80000008&
      Height          =   195
      Left            =   2160
      TabIndex        =   0
      Top             =   720
      Value           =   -1  'True
      Visible         =   0   'False
      Width           =   195
   End
   Begin VB.Menu mnuContext 
      Caption         =   "Context Form"
      Index           =   0
      Visible         =   0   'False
      Begin VB.Menu add1 
         Caption         =   "Add a point here"
      End
      Begin VB.Menu add5 
         Caption         =   "Add 5 Points"
      End
      Begin VB.Menu add10 
         Caption         =   "Add 10 Points"
      End
      Begin VB.Menu mnuRandom 
         Caption         =   "Random placement"
      End
      Begin VB.Menu mnuDummy1 
         Caption         =   "-"
      End
      Begin VB.Menu RemoveAll 
         Caption         =   "Remove all Points"
      End
      Begin VB.Menu mnuDummy 
         Caption         =   "-"
      End
      Begin VB.Menu mnuView 
         Caption         =   "View processing"
      End
      Begin VB.Menu mnuViewDist 
         Caption         =   "View distances"
      End
      Begin VB.Menu mnuAutoUpdate 
         Caption         =   "Auto update"
      End
   End
   Begin VB.Menu mnuContext 
      Caption         =   "Context Point"
      Index           =   1
      Visible         =   0   'False
      Begin VB.Menu Remove 
         Caption         =   "Remove"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'Public lCount As Long
Private OffsetX As Single, OffsetY As Single
Private hX As Single, hY As Single
Private PIndex As Long
Private PColor
Private theGraph As CGraph

Private Sub add1_Click()
    Addpoint
    BuildGraph
End Sub

Private Sub Addpoint()
    Dim i As Long
    i = Points.UBound + 1
    Load Points(i)
    With Points(i)
        .Left = OffsetX
        .Top = OffsetY
        .Visible = True
        .BackColor = PColor
    End With
    OffsetX = OffsetX + 3 * hX
    OffsetY = OffsetY + 3 * hY
End Sub

Private Sub add10_Click()
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    BuildGraph
End Sub

Private Sub add5_Click()
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    Addpoint
    BuildGraph
End Sub

Private Sub Form_DblClick()
    Debug.Print "Form_DblClick", OffsetX, OffsetY
    Addpoint
    BuildGraph
End Sub

Private Sub Form_Load()
    Dim i As Integer
    
    Randomize
    PColor = RGB(128, 128, 128)
    For i = 1 To 20
        Addpoint
    Next
    Random
    With Points(0)
        hX = .Width / 2
        hY = .Height / 2
    End With
    PIndex = -1
    Marker.Value = True
    Show
    BuildGraph
End Sub

Private Sub Form_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Debug.Print "Form_MouseDown", X, Y
    OffsetX = X
    OffsetY = Y
    If Button = vbRightButton Then
        PopupMenu mnuContext(0), vbPopupMenuRightButton + vbPopupMenuCenterAlign, X, Y, add1
    End If
End Sub

Private Sub Form_Paint()
    DrawGraph theGraph
End Sub

Private Sub mnuAutoUpdate_Click()
    Timer2.Enabled = Not Timer2.Enabled
End Sub

Private Sub mnuContext_Click(Index As Integer)
    mnuAutoUpdate.Checked = Timer2.Enabled
End Sub

Private Sub mnuRandom_Click()
    Random
    BuildGraph
End Sub

Private Sub Random()
    Dim p As Control
    
    On Error Resume Next
    For Each p In Points
        p.Move Rnd() * (ScaleWidth - 2 * hX), Rnd() * (ScaleHeight - 2 * hY)
    Next
    On Error GoTo 0
End Sub

Private Sub mnuView_Click()
    mnuView.Checked = Not mnuView.Checked
    If mnuView.Checked Then BuildGraph
End Sub

Private Sub mnuViewDist_Click()
    mnuViewDist.Checked = Not mnuViewDist.Checked
    DrawGraph theGraph
End Sub

Private Sub Points_MouseDown(Index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = vbLeftButton Then
        With Points(Index)
            Marker.Move .Left, .Top
            OffsetX = .Left - X
            OffsetY = .Top - Y
            Marker.Visible = True
            .BackColor = BackColor
        End With
        PIndex = Index
    Else
        Remove.Tag = Index
        PopupMenu mnuContext(1), vbPopupMenuRightButton + vbPopupMenuCenterAlign, Points(Index).Left + X, Points(Index).Top + Y
    End If
End Sub

Private Sub Points_MouseMove(Index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
    If PIndex = Index Then
        Marker.Move OffsetX + X, OffsetY + Y
        Timer1.Enabled = False
        Timer1.Interval = 100
        Timer1.Enabled = True
    End If
End Sub

Private Sub Points_MouseUp(Index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
    If PIndex = Index Then
        With Points(Index)
            .Move Marker.Left, Marker.Top
            Marker.Visible = False
            .BackColor = PColor
        End With
        PIndex = -1
    End If
End Sub

Private Sub Remove_Click()
    Unload Points(Remove.Tag)
    BuildGraph
End Sub

Private Sub RemoveAll_Click()
    Dim p As Control
    For Each p In Points
        If Not p Is Points(0) Then Unload p
    Next
    BuildGraph
End Sub

Private Sub Timer1_Timer()
    Timer1.Enabled = False
    BuildGraph
End Sub

Private Sub BuildGraph()
    Dim G As CGraph
    Dim p As Control
    
    Screen.MousePointer = vbHourglass
    Set G = New CGraph
    Set theGraph = G
    With G.Ecken
        For Each p In Points
            If Not p Is Points(0) Then
                If p.Index = PIndex Then
                    .Add Marker.Left, Marker.Top
                Else
                    .Add p.Left, p.Top
                End If
            End If
        Next
    End With
    G.Vollstaendig
    G.Geruest mnuView.Checked
    DrawGraph G
    Screen.MousePointer = 0
End Sub

Public Sub DrawGraph(ByVal G As CGraph)
    Dim K As CKante
    
    If G Is Nothing Then Exit Sub
    frmMain.Cls
    For Each K In G.Kanten
        DrawKante K
    Next
End Sub

Public Sub DrawKante(K As CKante, Optional ByVal Farbe = vbBlack)
    Line (K.E1.X + hX, K.E1.Y + hY)-(K.E2.X + hX, K.E2.Y + hY), Farbe
    If theGraph.Kanten.Count < 40 And mnuViewDist.Checked Then
        CurrentX = (K.E1.X + K.E2.X) / 2
        CurrentY = (K.E1.Y + K.E2.Y) / 2
        Print Int(K.Distance)
    End If
End Sub

Private Sub Timer2_Timer()
    Random
    BuildGraph
End Sub
