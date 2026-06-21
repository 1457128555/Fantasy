#!/usr/bin/env bash
# 一键：生成工程 -> 编译(模拟器) -> 启动模拟器 -> 装 -> 跑 -> 截图
# 用法：./verify.sh [输出png名]   默认 screenshot.png
#       SIM_NAME / RUNTIME 可用环境变量覆盖（spike 测老 runtime 用）
set -euo pipefail
cd "$(dirname "$0")"

SCHEME="Fantasy"
BUNDLE_ID="com.cfan.fantasy"
SIM_NAME="${SIM_NAME:-iPhone 17 Pro}"
DERIVED="build"
SHOT="${1:-screenshot.png}"

echo "==> [1/6] xcodegen 生成工程"
xcodegen generate >/dev/null

echo "==> [2/6] 解析模拟器 UDID: ${SIM_NAME}"
UDID=$(xcrun simctl list devices available \
        | grep -F "$SIM_NAME (" | head -1 \
        | sed -E 's/.*\(([0-9A-Fa-f-]{36})\).*/\1/')
if [ -z "$UDID" ]; then echo "找不到模拟器: $SIM_NAME"; exit 1; fi
echo "    UDID=$UDID"

echo "==> [3/6] 编译（Debug / iphonesimulator）"
xcodebuild \
  -project Fantasy.xcodeproj \
  -scheme "$SCHEME" \
  -configuration Debug \
  -destination "id=$UDID" \
  -derivedDataPath "$DERIVED" \
  build 2>&1 | tee build.log | tail -15

APP_PATH="$DERIVED/Build/Products/Debug-iphonesimulator/$SCHEME.app"
if [ ! -d "$APP_PATH" ]; then echo "没找到产物: ${APP_PATH} (看 build.log)"; exit 1; fi

echo "==> [4/6] 启动模拟器"
xcrun simctl boot "$UDID" 2>/dev/null || true
xcrun simctl bootstatus "$UDID" >/dev/null 2>&1 || true

echo "==> [5/6] 安装 + 启动 app"
xcrun simctl install "$UDID" "$APP_PATH"
xcrun simctl launch "$UDID" "$BUNDLE_ID" || true
sleep 3

echo "==> [6/6] 截图 -> $SHOT"
xcrun simctl io "$UDID" screenshot "$SHOT"
echo "DONE: $(pwd)/$SHOT"
