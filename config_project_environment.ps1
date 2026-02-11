# Run “Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force” in powershell to Resolve the issue of script execution failure

Param
(
    [string]$Qt6_Path = "D:\Library\Qt\6.8.3\msvc2022_64\"
)

# 判断当前是否管理员权限
if (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    # 获取当前脚本路径
    $scriptPath = $MyInvocation.MyCommand.Definition
    # 以管理员身份启动新进程并传递参数
    Start-Process powershell.exe -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`""
    Exit  # 退出当前非管理员进程
}
# 后续代码将以管理员权限执行
Write-Host "Currently have administrator privileges!" -ForegroundColor Green

[System.Environment]::SetEnvironmentVariable("CMAKE_PREFIX_PATH", "$Qt6_Path", "Machine")