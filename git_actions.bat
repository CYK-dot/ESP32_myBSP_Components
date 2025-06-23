@echo off
setlocal enabledelayedexpansion

:menu
cls
echo ===============================================
echo               Git 助手菜单
echo ===============================================
echo 1. 展示所有分支及其提交信息
echo 2. 展示当前分支上的 commit 列表
echo -----------------------------------------------
echo 3. 添加所有更改到暂存区 (git add .)
echo 4. 提交更改 (用户输入 message)
echo -----------------------------------------------
echo 5. 回滚到某 commit，并创建新分支
echo 6. 切换到某分支（可先 commit 或丢弃更改）
echo 7. 退出
echo ===============================================
set /p choice=请输入选项（1-7）: 

if "%choice%"=="1" (
    echo 显示所有分支及其 HEAD commit message：
    for /f "tokens=*" %%b in ('git branch --format="%%(refname:short)"') do (
        for /f "tokens=*" %%c in ('git log -1 --pretty=format:"%%s" %%b') do (
            echo [%%b] - %%c
        )
    )
    goto pauseAndMenu
)

if "%choice%"=="2" (
    echo 当前分支 commit 历史：
    git log --oneline --graph --decorate
    goto pauseAndMenu
)

if "%choice%"=="3" (
    git add .
    echo 所有更改已加入暂存区
    goto pauseAndMenu
)

if "%choice%"=="4" (
    set /p msg=请输入 commit message: 
    git commit -m "!msg!"
    goto pauseAndMenu
)

if "%choice%"=="5" (
    git log --oneline
    set /p cid=请输入要回滚到的 commit id（前几位即可）:
    set /p newbranch=请输入新分支名称:
    git checkout !cid!
    git checkout -b !newbranch!
    echo 已切换到新分支 !newbranch!，HEAD 位于 !cid!
    goto pauseAndMenu
)

if "%choice%"=="6" (
    git status
    echo 当前有未提交更改时，切换分支可能导致数据丢失
    echo 1. 先 git commit 再切换
    echo 2. 丢弃所有更改后切换
    echo 3. 直接尝试切换（若失败需手动解决）
    set /p action=请选择处理方式（1-3）:
    set /p tgtbranch=请输入目标分支名:

    if "!action!"=="1" (
        set /p msg=请输入 commit message:
        git add .
        git commit -m "!msg!"
        git checkout !tgtbranch!
    ) else if "!action!"=="2" (
        git reset --hard
        git checkout !tgtbranch!
    ) else (
        git checkout !tgtbranch!
    )
    goto pauseAndMenu
)

if "%choice%"=="7" (
    goto end
)

echo 无效选项
goto pauseAndMenu

:pauseAndMenu
echo.
pause
goto menu

:end
endlocal
