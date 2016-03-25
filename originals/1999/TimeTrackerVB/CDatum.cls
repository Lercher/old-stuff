VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CDatum"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Public asDate As Date

Public Function asString() As String
    If asDouble > 0 Then
        asString = Format$(asDate, "dd/mm/yy Hh:Nn")
    Else
        asString = ""
    End If
End Function

Public Function asDouble() As Double
    asDouble = CDbl(asDate)
End Function

Public Sub fromDouble(ByVal aDouble As Double)
    asDate = CDate(aDouble)
End Sub

Public Function TimeTo(ByVal aDatum As CDatum) As Date
    TimeTo = aDatum.asDate - asDate
End Function

Public Sub SetNow()
    asDate = Now()
End Sub

Public Sub SetNull()
    fromDouble 0
End Sub

Public Function isEqual(ByVal d As CDatum) As Boolean
    isEqual = (Int(asDate) = Int(d.asDate))
End Function

Public Function isNull() As Boolean
    isNull = (asDouble = 0)
End Function
