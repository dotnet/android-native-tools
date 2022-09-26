$UpxVer="3.96"

$progresspreference = 'silentlyContinue'
Invoke-WebRequest https://github.com/upx/upx/releases/download/v$UpxVer/upx-3.96-win64.zip -OutFile upx.zip
$progressPreference = 'Continue'

$7zipPath="C:\Program Files\7-Zip\7z.exe"
Set-Alias 7z "$7zipPath"

7z x upx.zip -oupx
copy upx\upx-$UpxVer-win64\upx.exe .
