<#
Applies database/migrations/V*.sql (and optionally seeds/S*.sql) in order and records
what was applied in schema_migrations, so each file runs only once.

Usage (PowerShell, from the repo root):
  .\database\apply_migrations.ps1 -MysqlExe "C:\Program Files\MySQL\MySQL Server 8.4\bin\mysql.exe" -Database cold_storage -User root -Seeds

The password is asked interactively and never written to disk. Do not commit passwords.
#>
param(
    [Parameter(Mandatory = $true)][string]$MysqlExe,
    [Parameter(Mandatory = $true)][string]$Database,
    [string]$User = "root",
    [string]$HostName = "localhost",
    [switch]$Seeds
)

$ErrorActionPreference = "Stop"
$secure = Read-Host "MySQL password for $User" -AsSecureString
$bstr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure)
$env:MYSQL_PWD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($bstr)

function Invoke-Sql([string]$sql) {
    $sql | & $MysqlExe --host=$HostName --user=$User --default-character-set=utf8mb4 --batch --skip-column-names $Database
    if ($LASTEXITCODE -ne 0) { throw "mysql failed" }
}
function Invoke-SqlFile([string]$path) {
    Get-Content -Raw -Encoding UTF8 $path | & $MysqlExe --host=$HostName --user=$User --default-character-set=utf8mb4 $Database
    if ($LASTEXITCODE -ne 0) { throw "mysql failed on $path" }
}

try {
    Invoke-Sql "SET time_zone = '+00:00';"   # connectivity check
    Invoke-Sql "CREATE TABLE IF NOT EXISTS schema_migrations (filename VARCHAR(255) NOT NULL PRIMARY KEY, applied_at DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3));"
    $applied = @(Invoke-Sql "SELECT filename FROM schema_migrations;")

    $files = @(Get-ChildItem (Join-Path $PSScriptRoot "migrations") -Filter "V*.sql" | Sort-Object Name)
    if ($Seeds) { $files += @(Get-ChildItem (Join-Path $PSScriptRoot "seeds") -Filter "S*.sql" | Sort-Object Name) }

    foreach ($f in $files) {
        if ($applied -contains $f.Name) { Write-Host "skip   $($f.Name)"; continue }
        Write-Host "apply  $($f.Name)"
        Invoke-SqlFile $f.FullName
        Invoke-Sql "INSERT INTO schema_migrations (filename) VALUES ('$($f.Name)');"
    }
    Write-Host "Done."
}
finally {
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
}
