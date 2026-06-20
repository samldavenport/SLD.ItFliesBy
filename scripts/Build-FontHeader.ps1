param(
    [string]$InputFile,
    [string]$OutputHeader
)

function Sanitize-VariableName {
    param([string]$Name)

    $Result = $Name -replace '[^a-zA-Z0-9]', '_'

    if ($Result -match '^[0-9]') {
        $Result = '_' + $Result
    }

    return $Result
}

$Bytes = [System.IO.File]::ReadAllBytes($InputFile)

$VariableName =
    Sanitize-VariableName(
        [System.IO.Path]::GetFileNameWithoutExtension($InputFile)
    )

$Indent = "`t"
$ByteLines = @()

# Emit 16 bytes per line
for ($i = 0; $i -lt $Bytes.Length; $i += 16)
{
    $LineBytes = @()

    for ($j = $i; $j -lt ($i + 16) -and $j -lt $Bytes.Length; $j++)
    {
        $LineBytes += ("0x{0:X2}" -f $Bytes[$j])
    }

    $ByteLines += "$Indent$Indent" +
                  ($LineBytes -join ", ") +
                  ","
}

$Header = @"
#pragma once

namespace ifb
{
    static const unsigned char $VariableName[] =
    {
$($ByteLines -join "`n")
    };

    static const unsigned int ${VariableName}_size =
        sizeof($VariableName);
}
"@

Set-Content -Path $OutputHeader -Value $Header

Write-Host "Generated: $OutputHeader"