param(
    [Parameter(Mandatory)] [string] $OutFile,
    [string] $Map = "",
    [int] $WaitSec = 30,
    [string] $Clicks = "",
    [string] $PreShot = "",
    [int] $ClickWait = 6,
    [string] $Engine = "C:\Program Files\Epic Games\UE_5.6\UE_5.6"
)
# 게임을 -game 창모드(1600x900)로 띄워 WaitSec 후 창을 캡처하고 종료한다. 에디터 작업 없이 UI 확인용.
#
#   타이틀:  .\tools\ui\capture.ps1 -OutFile title.png
#   로비:    .\tools\ui\capture.ps1 -OutFile lobby.png -Clicks "1500,450;470,800"   (빈 곳으로 포커스 → 싱글 플레이)
#   인게임:  .\tools\ui\capture.ps1 -OutFile hud.png -Map /Game/Maps/LV_Island
#
# -Clicks는 실제 마우스로 클릭한다(클릭 순간만 게임 창을 맨 위로 올리고, 커서는 원위치). 캡처 중엔 마우스를 쓰지 말 것.
$ErrorActionPreference = "Stop"
$uproject = Join-Path $PSScriptRoot "..\..\ChapSalKing\ChapSalKing.uproject"
$exe = Join-Path $Engine "Engine\Binaries\Win64\UnrealEditor.exe"

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @"
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
public static class CskShot {
  [DllImport("user32.dll")] static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h, IntPtr pid);
  [DllImport("kernel32.dll")] static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] static extern bool AttachThreadInput(uint a, uint b, bool attach);
  [DllImport("user32.dll")] static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [DllImport("user32.dll")] static extern bool GetCursorPos(out POINT p);
  [DllImport("user32.dll")] static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [DllImport("user32.dll")] static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] static extern bool IsIconic(IntPtr h);
  struct RECT { public int L, T, R, B; }
  struct POINT { public int X, Y; }
  // keep the game window on top only while clicking so the click can't land on another app
  public static void Click(IntPtr h, int x, int y) {
    SetWindowPos(h, (IntPtr)(-1), 0, 0, 0, 0, 0x0001 | 0x0002 | 0x0040);
    IntPtr fg = GetForegroundWindow();
    uint fgThread = GetWindowThreadProcessId(fg, IntPtr.Zero);
    uint me = GetCurrentThreadId();
    AttachThreadInput(me, fgThread, true);
    BringWindowToTop(h);
    SetForegroundWindow(h);
    AttachThreadInput(me, fgThread, false);
    System.Threading.Thread.Sleep(300);
    POINT old; GetCursorPos(out old);
    POINT p; p.X = x; p.Y = y; ClientToScreen(h, ref p);
    SetCursorPos(p.X, p.Y);
    System.Threading.Thread.Sleep(200);
    mouse_event(0x0002, 0, 0, 0, IntPtr.Zero);
    System.Threading.Thread.Sleep(100);
    mouse_event(0x0004, 0, 0, 0, IntPtr.Zero);
    System.Threading.Thread.Sleep(150);
    SetCursorPos(old.X, old.Y);
    SetWindowPos(h, (IntPtr)(-2), 0, 0, 0, 0, 0x0001 | 0x0002);
  }
  // PW_CLIENTONLY | PW_RENDERFULLCONTENT: a covered DirectX window still captures
  public static string Capture(IntPtr h, string path) {
    if (IsIconic(h)) { ShowWindow(h, 4); System.Threading.Thread.Sleep(1500); }
    RECT r; GetClientRect(h, out r);
    int w = r.R - r.L, hh = r.B - r.T;
    using (var bmp = new Bitmap(w, hh))
    using (var g = Graphics.FromImage(bmp)) {
      IntPtr hdc = g.GetHdc();
      PrintWindow(h, hdc, 3);
      g.ReleaseHdc(hdc);
      bmp.Save(path, ImageFormat.Png);
    }
    return w + "x" + hh;
  }
}
"@
[CskShot]::SetProcessDPIAware() | Out-Null

$argList = @("`"$uproject`"")
if ($Map) { $argList += $Map }
$argList += @("-game", "-windowed", "-ResX=1600", "-ResY=900", "-nosplash", "-NoSound")
$proc = Start-Process -FilePath $exe -ArgumentList $argList -PassThru

$deadline = (Get-Date).AddSeconds(180)
while ($proc.MainWindowHandle -eq 0 -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 500; $proc.Refresh() }
Start-Sleep -Seconds $WaitSec
$proc.Refresh()
if ($PreShot) { "pre: " + [CskShot]::Capture($proc.MainWindowHandle, $PreShot) }
foreach ($c in ($Clicks -split ';' | Where-Object { $_ })) {
    $xy = $c -split ','
    [CskShot]::Click($proc.MainWindowHandle, [int]$xy[0], [int]$xy[1])
    Start-Sleep -Seconds $ClickWait
}
"captured " + [CskShot]::Capture($proc.MainWindowHandle, $OutFile) + " -> $OutFile"
Stop-Process -Id $proc.Id -Force
