<#
End-to-end smoke test of the API (login, products, cold rooms, compatibility, auth checks).
The server must be running. Run it from the repo root:
  .\backend\tests\smoke_test.ps1
It asks for the admin password (hidden) and creates test records with a unique suffix, so it can be re-run.
#>
param(
    [string]$BaseUrl = "http://localhost:8081/api/v1",
    [string]$Username = "admin"
)

$script:Headers = @{}
$script:Failures = 0

function Call([string]$Method, [string]$Path, $Body = $null) {
    $params = @{ Method = $Method; Uri = "$BaseUrl$Path"; Headers = $script:Headers; ContentType = "application/json" }
    if ($null -ne $Body) { $params.Body = ($Body | ConvertTo-Json -Depth 5) }
    try {
        $r = Invoke-RestMethod @params
        return [pscustomobject]@{ status = 200; body = $r }
    } catch {
        $resp = $_.Exception.Response
        if ($null -eq $resp) { throw }
        $status = [int]$resp.StatusCode
        $text = (New-Object System.IO.StreamReader($resp.GetResponseStream())).ReadToEnd()
        $parsed = $null
        try { $parsed = $text | ConvertFrom-Json } catch {}
        return [pscustomobject]@{ status = $status; body = $parsed }
    }
}

function Check([string]$Name, [bool]$Condition) {
    if ($Condition) { Write-Host "PASS  $Name" -ForegroundColor Green }
    else { Write-Host "FAIL  $Name" -ForegroundColor Red; $script:Failures++ }
}

$sec = Read-Host "Password for $Username" -AsSecureString
$pw = [System.Net.NetworkCredential]::new("", $sec).Password
$sfx = Get-Date -Format "HHmmss"

# --- auth ---
$r = Call "POST" "/auth/login" @{ username = $Username; password = $pw }
Check "login" ($r.status -eq 200 -and $r.body.success)
if ($r.status -ne 200) { Write-Host "Cannot continue without a login." -ForegroundColor Red; exit 1 }
$script:Headers = @{ Authorization = "Bearer $($r.body.data.token)" }

$saved = $script:Headers; $script:Headers = @{}
$r = Call "GET" "/products"
Check "products without token -> 401" ($r.status -eq 401)
$script:Headers = $saved

# --- products ---
$r = Call "POST" "/products" @{ code = "P$sfx"; name = "Test tomatoes $sfx"; min_temp_c = 8; max_temp_c = 12; min_humidity_pct = 85; max_humidity_pct = 95; max_storage_days = 30 }
Check "create product -> 201" ($r.status -eq 200 -and $r.body.data.code -eq "P$sfx")
$productId = $r.body.data.id

$r = Call "POST" "/products" @{ code = "PX$sfx"; name = "Bad"; min_temp_c = 10; max_temp_c = 2 }
Check "product min_temp > max_temp -> 422" ($r.status -eq 422)

$r = Call "POST" "/products" @{ code = "P$sfx"; name = "Duplicate" ; min_temp_c = 1; max_temp_c = 2 }
Check "duplicate product code -> 409" ($r.status -eq 409)

$r = Call "PATCH" "/products/$productId" @{ max_storage_days = 45 }
Check "patch product (only max_storage_days)" ($r.status -eq 200 -and $r.body.data.max_storage_days -eq 45 -and $r.body.data.name -eq "Test tomatoes $sfx")

$r = Call "PATCH" "/products/$productId" @{ min_temp_c = 50 }
Check "patch product with invalid result -> 422" ($r.status -eq 422)

$r = Call "GET" "/products/999999"
Check "unknown product -> 404" ($r.status -eq 404)

$r = Call "GET" "/products?q=$sfx"
Check "search products" ($r.status -eq 200 -and $r.body.data.total -ge 1)

# --- cold rooms ---
$r = Call "POST" "/cold-rooms" @{ code = "R$sfx"; name = "Room $sfx"; capacity_kg = 5000.5; min_temp_c = 0; max_temp_c = 15 }
Check "create cold room" ($r.status -eq 200 -and $r.body.data.capacity_kg -eq 5000.5)
$roomId = $r.body.data.id

$r = Call "POST" "/cold-rooms" @{ code = "RX$sfx"; name = "Bad"; capacity_kg = 0; min_temp_c = 0; max_temp_c = 5 }
Check "capacity 0 -> 422" ($r.status -eq 422)

$r = Call "PATCH" "/cold-rooms/$roomId" @{ capacity_kg = 6000 }
Check "patch cold room capacity" ($r.status -eq 200 -and $r.body.data.capacity_kg -eq 6000)

# --- compatibility ---
$r = Call "PUT" "/cold-rooms/$roomId/products" @{ product_ids = @($productId) }
Check "set compatible products" ($r.status -eq 200 -and @($r.body.data.product_ids).Count -eq 1)

$r = Call "PUT" "/cold-rooms/$roomId/products" @{ product_ids = @(999999) }
Check "unknown product id -> 422" ($r.status -eq 422)

$r = Call "GET" "/cold-rooms/$roomId/products"
Check "compatibility unchanged after the failed update" ($r.status -eq 200 -and @($r.body.data.product_ids).Count -eq 1)

$r = Call "PUT" "/cold-rooms/$roomId/products" @{ product_ids = @() }
Check "clear compatible products" ($r.status -eq 200 -and @($r.body.data.product_ids).Count -eq 0)

$r = Call "GET" "/cold-rooms"
Check "list cold rooms" ($r.status -eq 200 -and $r.body.data.total -ge 1)

Write-Host ""
if ($script:Failures -eq 0) { Write-Host "All checks passed." -ForegroundColor Green }
else { Write-Host "$($script:Failures) check(s) failed." -ForegroundColor Red; exit 1 }
