$ARTIFACTS_DIR = "${PSScriptRoot}/artifacts"
$ANDROID_TRIPLES = "aarch64-linux-android arm-linux-androideabi i686-linux-android x86_64-linux-android"

function Make-PrefixedBinaries {
    param (
        [string] $binary
    )

    $ANDROID_TRIPLES -split ' ' | ForEach-Object {
        Write-Output "$_-${binary}"
    }
}

$LLVM_BINARIES = "llvm-mc llvm-strip lld llc"
$LLVM_PREFIXED_BINARIES = "$(Make-PrefixedBinaries "strip") $(Make-PrefixedBinaries "ld")"
$XA_UTILS_BINARIES = "as"
$XA_UTILS_PREFIXED_BINARIES = Make-PrefixedBinaries "as"
$BINARIES = "${LLVM_BINARIES} ${LLVM_PREFIXED_BINARIES} ${XA_UTILS_BINARIES} ${XA_UTILS_PREFIXED_BINARIES}"
$DIST_PACKAGE_NAME_BASE = "xamarin-android-toolchain"
$OPERATING_SYSTEMS="linux darwin windows"

$PackageTreeDir = "${ARTIFACTS_DIR}/package"
$PackageArtifactsDir = "${PackageTreeDir}/artifacts"

$LLVMVersion = ""

function Make-Windows-Wrapper-Scripts {
    param (
        [string] $inputScript,
        [string] $outputDir,
        [string] $outputBaseName
    )

    foreach ($triple in $ANDROID_TRIPLES -split ' ') {
        $outputName = "${triple}-${outputBaseName}"
        $outputPath = "${outputDir}/${outputName}.cmd"
        (Get-Content $inputScript) -replace '@TARGET_NAME@', $outputName | Set-Content $outputPath
    }
}

function Make-Unix-Wrapper-Scripts {
    param (
        [string] $inputScript,
        [string] $outputDir,
        [string] $outputBaseName
    )

    foreach ($triple in $ANDROID_TRIPLES -split ' ') {
        $outputName = "${triple}-${outputBaseName}"
        $outputPath = "${outputDir}/${outputName}"
        Copy-Item $inputScript $outputPath -Force
    }
}

function Prepare {
    param (
        [string] $os
    )

    $artifactsSource = "${ARTIFACTS_DIR}/${os}/"
    $artifactsSourceBin = "${artifactsSource}/bin"
    $artifactsSourceLib = "${artifactsSource}lib"
    $artifactsDestBin = "${PackageArtifactsDir}/${os}/bin"
    $artifactsDestLib = "${PackageArtifactsDir}/${os}/lib"

    Write-Host "`nPreparing binaries for $os`n"

    New-Item -ItemType Directory -Force -Path $artifactsDestBin
    New-Item -ItemType Directory -Force -Path $artifactsDestLib

    if ($os -eq "windows") {
        $exe = ".exe"
        $cmd = ".cmd"

        Make-Windows-Wrapper-Scripts "scripts/llvm-strip.cmd.in" $artifactsSourceBin "strip"
        Make-Windows-Wrapper-Scripts "scripts/gas.cmd.in" $artifactsSourceBin "as"
        Make-Windows-Wrapper-Scripts "scripts/ld.cmd.in" $artifactsSourceBin "ld"
    }
    else {
        Make-Unix-Wrapper-Scripts "scripts/llvm-strip.sh" $artifactsSourceBin "strip"
        Make-Unix-Wrapper-Scripts "scripts/gas.sh" $artifactsSourceBin "as"
        Make-Unix-Wrapper-Scripts "scripts/ld.sh" $artifactsSourceBin "ld"
    }

    if ([string]::IsNullOrEmpty($LLVMVersion)) {
        $versionFile = "${artifactsSource}/llvm-version.txt"
        $script:LLVMVersion = Get-Content $versionFile | Select-Object -First 1
        Copy-Item $versionFile $PackageArtifactsDir
    }

    foreach ($b in $BINARIES -split ' ') {
        Write-Host "Preparing '${b}'"
        $srcPdb = ""
        $destPdb = ""
        if ($b -eq "lld") {
            $destB = "ld"
        }
        else {
            $destB = $b
        }

        if (Test-Path "${artifactsSourceBin}/${b}${exe}") {
            $srcPdb = "${b}.pdb"
            $destPdb = "${destB}.pdb"
            $b = "${b}${exe}"
            $destB = "${destB}${exe}"
        }
        elseif (Test-Path "${artifactsSourceBin}/${b}${cmd}") {
            $b = "${b}${cmd}"
            $destB = "${destB}${cmd}"
        }

        if (Test-Path "${artifactsSourceBin}/${b}.upx") {
            Copy-Item "${artifactsSourceBin}/${b}.upx" "${artifactsDestBin}/${destB}" -Force
        }
        else {
            Copy-Item "${artifactsSourceBin}/${b}" "${artifactsDestBin}/${destB}" -Force
        }

        if (![string]::IsNullOrEmpty($srcPdb) -and (Test-Path "${artifactsSourceBin}/${srcPdb}")) {
            Copy-Item "${artifactsSourceBin}/${srcPdb}" "${artifactsDestBin}/${destPdb}" -Force
        }
    }

    if (Test-Path $artifactsSourceLib) {
        Copy-Item "${artifactsSourceLib}/*" $artifactsDestLib -Force
        Get-ChildItem $artifactsDestLib | ForEach-Object { $_.Attributes = 'Normal' }
    }
}

foreach ($os in $OPERATING_SYSTEMS -split ' ') {
    Prepare $os
}

if ([string]::IsNullOrEmpty($LLVMVersion)) {
    throw "Unable to detect LLVM version from the artifacts"
}

Write-Host "Prepared package for LLVM version ${LLVMVersion}"
