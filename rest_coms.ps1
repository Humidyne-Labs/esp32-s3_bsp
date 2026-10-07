# ==============================================================================
# reset_coms.ps1 - Rescan and reset ESP32 / USB Serial interfaces in Windows
# ==============================================================================

# 1. Ensure the script runs with Administrator privileges
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator
)

if (-not $isAdmin) {
    Write-Host "[!] Admin rights required to reset hardware drivers. Elevating..." -ForegroundColor Yellow
    Start-Process powershell.exe -Verb RunAs -ArgumentList ("-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"")
    exit
}

Write-Host "=== Windows Serial & USB Bus Reset Tool ===" -ForegroundColor Cyan

# 2. Trigger hardware scan for Plug and Play changes
Write-Host "[*] Triggering Windows PnP bus rescan..." -ForegroundColor Cyan
pnputil /scan-devices | Out-Null
Start-Sleep -Milliseconds 600

# 3. Locate connected (Present = True) target devices
$devices = Get-PnpDevice -PresentOnly | Where-Object {
    ($_.Class -in @('Ports', 'USB')) -and
    ($_.FriendlyName -match 'Espressif|ESP32|CP210|CH34|USB Serial Device|JTAG')
}

if (-not $devices) {
    Write-Host "`n[!] No active ESP32 or USB-Serial devices detected on the bus." -ForegroundColor Red
    Write-Host "    If using ESP32-S3 native USB:" -ForegroundColor Yellow
    Write-Host "    1. Hold BOOT (GPIO 0)"
    Write-Host "    2. Press and release RESET (EN)"
    Write-Host "    3. Release BOOT"
    Write-Host "    4. Run this script again once Windows plays the connect chime.`n"
} else {
    Write-Host "`n[*] Found $($devices.Count) connected device(s):" -ForegroundColor Green
    foreach ($dev in $devices) {
        Write-Host "  -> $($dev.FriendlyName) [$($dev.InstanceId)]" -ForegroundColor Gray
    }

    # 4. Restart each device instance individually using pnputil
    Write-Host "`n[*] Restarting driver stack(s)..." -ForegroundColor Cyan
    foreach ($dev in $devices) {
        Write-Host "  Restarting: $($dev.FriendlyName)..." -NoNewline
        $result = pnputil /restart-device "$($dev.InstanceId)" 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host " [OK]" -ForegroundColor Green
        } else {
            Write-Host " [FAILED]" -ForegroundColor Red
            Write-Host "    $result" -ForegroundColor DarkRed
        }
    }
}

# 5. Output active COM ports currently registered in Windows
Start-Sleep -Milliseconds 500
Write-Host "`n[*] Currently registered COM ports:" -ForegroundColor Cyan
$activePorts = [System.IO.Ports.SerialPort]::GetPortNames()

if ($activePorts) {
    foreach ($port in ($activePorts | Sort-Object)) {
        Write-Host "  [+] $port" -ForegroundColor Green
    }
} else {
    Write-Host "  [-] No active COM ports registered." -ForegroundColor Yellow
}

Write-Host "`nDone." -ForegroundColor Cyan