param([string] $Path)

function Get-DumpBinPath($Arch = 'x64', $HostArch = 'x64') {
    $InstallDir = & "$PSScriptRoot\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($InstallDir) {
        $Path = Join-Path $installDir 'VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt'
        if (Test-Path $Path) {
            $Version = Get-Content -raw $Path
            if ($Version) {
                $Version = $version.Trim()
                $Path = Join-Path $installDir "VC\Tools\MSVC\$version\bin\Host$HostArch\$Arch\dumpbin.exe"
                return $Path
            }
        }
    }

    # Visual Studio 2015
    if (Test-Path "${env:ProgramFiles(x86)}\Microsoft Visual Studio 14.0\VC\bin\dumpbin.exe") {
        return "${env:ProgramFiles(x86)}\Microsoft Visual Studio 14.0\VC\bin\dumpbin.exe"
    }

    return $null
}

$ErrorActionPreference = "Stop"

if ($null -eq $Path) {
    Write-Error "A filename or path must be provided."
}

if (!(Test-Path $Path)) {
    Write-Error "The specified filename does not exist: $Path"
}

$FullPath = (Resolve-Path -Path $Path).ToString()

$BannedInstructions = @(
    "F2XM1",
    "FABS",
    "FADD",
    "FADDP",
    "FBLD",
    "FBSTP",
    "FCHS",
    "FCLEX",
    "FCOM",
    "FCOMP",
    "FCOMPP",
    "FDECSTP",
    "FDISI",
    "FDIV",
    "FDIVP",
    "FDIVR",
    "FDIVRP",
    "FENI",
    "FFREE",
    "FIADD",
    "FICOM",
    "FICOMP",
    "FIDIV",
    "FIDIVR",
    "FILD",
    "FIMUL",
    "FINCSTP",
    "FINIT",
    "FIST",
    "FISTP",
    "FISUB",
    "FISUBR",
    "FLD",
    "FLD1",
    "FLDCW",
    "FLDENV",
    "FLDENVW",
    "FLDL2E",
    "FLDL2T",
    "FLDLG2",
    "FLDLN2",
    "FLDPI",
    "FLDZ",
    "FMUL",
    "FMULP",
    "FNCLEX",
    "FNDISI",
    "FNENI",
    "FNINIT",
    "FNOP",
    "FNSAVE",
    "FNSAVEW",
    "FNSTCW",
    "FNSTENV",
    "FNSTENVW",
    "FNSTSW",
    "FPATAN",
    "FPREM",
    "FPTAN",
    "FRNDINT",
    "FRSTOR",
    "FRSTORW",
    "FSAVE",
    "FSAVEW",
    "FSCALE",
    "FSQRT",
    "FST",
    "FSTCW",
    "FSTENV",
    "FSTENVW",
    "FSTP",
    "FSTSW",
    "FSUB",
    "FSUBP",
    "FSUBR",
    "FSUBRP",
    "FTST",
    "FWAIT",
    "FXAM",
    "FXCH",
    "FXTRACT",
    "FYL2X",
    "FYL2XP1",
    "FSETPM",
    "FCOS",
    "FLDENVD",
    "FSAVED",
    "FSTENVD",
    "FPREM1",
    "FRSTORD",
    "FSIN",
    "FSINCOS",
    "FSTENVD",
    "FUCOM",
    "FUCOMP",
    "FUCOMPP"
)

$BannedRegex = [regex] ('(?m)^.+\s(' + [System.String]::Join("|", ($BannedInstructions | ForEach-Object { $_.ToLowerInvariant() })) + ')\s.+$')

if (($PSVersionTable.PSVersion.Major -eq 5 -or $IsWindows) -and ($FullPath.EndsWith(".exe") -or $FullPath.EndsWith(".dll"))) {
    $DumpBin = Get-DumpBinPath
    if ($null -eq $DumpBin) {
        Write-Error "Unable to locate dumpbin, make sure you have installed the Desktop C++ workload in Visual Studio."
    }

    $Process = New-Object System.Diagnostics.Process
    $Process.StartInfo.UseShellExecute = $false
    $Process.StartInfo.FileName = $DumpBin
    $Process.StartInfo.Arguments = @(
        "/DISASM",
        "`"$FullPath`""
    )
    $Process.StartInfo.CreateNoWindow = $true
    $Process.StartInfo.RedirectStandardOutput = $true

    [void]$Process.Start();

    $Output = $Process.StandardOutput.ReadToEnd();

    if ($Process.ExitCode -ne 0) {
        Write-Output "error: dumpbin exited with non-zero exit code."
        Write-Output ""
        Write-Output $Output

        exit 1
    }

    $Matches = $BannedRegex.Matches($Output)

    if ($Matches.Count -gt 0) {
        Write-Output "error: Found $($Matches.Count) x87 FPU instructions in: $FullPath"

        foreach ($Match in $Matches) {
            Write-Output "error: x87 FPU instruction detected at: $Match"
        }

        exit 1
    }

    Write-Output "info: No x87 FPU instructions found in: $FullPath"

    exit 0
}
elseif ($FullPath.EndsWith(".exe") -or $FullPath.EndsWith(".dll")) {
    Write-Error "fpcheck can not check .exe/.dll files on non-Windows platforms."
}
else {
    $ObjDump = Get-Command -Name "objdump"
    if ($null -eq $ObjDump) {
        if ($PSVersionTable.PSVersion.Major -eq 5 -or $IsWindows) {
            Write-Error "fpcheck can only check executable and DLL files on Windows, unless you install objdump (via Chocolately or WSL)."
        }
        else {
            Write-Error "fpcheck requires the objdump utility to be installed."
        }
    }

    $Process = New-Object System.Diagnostics.Process
    $Process.StartInfo.UseShellExecute = $false
    $Process.StartInfo.FileName = $ObjDump.Source
    $Process.StartInfo.Arguments = @(
        "-d",
        "`"$FullPath`""
    )
    $Process.StartInfo.CreateNoWindow = $true
    $Process.StartInfo.RedirectStandardOutput = $true

    [void]$Process.Start();

    $Output = $Process.StandardOutput.ReadToEnd();

    if ($Process.ExitCode -ne 0) {
        Write-Output "error: objdump exited with non-zero exit code."
        Write-Output ""
        Write-Output $Output

        exit 1
    }

    $Matches = $BannedRegex.Matches($Output)

    if ($Matches.Count -gt 0) {
        Write-Output "error: Found $($Matches.Count) x87 FPU instructions in: $FullPath"

        foreach ($Match in $Matches) {
            Write-Output "error: x87 FPU instruction detected at: $Match"
        }

        exit 1
    }

    Write-Output "info: No x87 FPU instructions found in: $FullPath"
}