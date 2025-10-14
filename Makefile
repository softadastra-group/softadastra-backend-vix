# =============================================================
# 🚀 Softadastra Backend (Vix.cpp App)
# =============================================================
# High-performance backend powered by Vix.cpp
# Build / Run / Release helper for all platforms
#
# Usage:
#   make build               → configure + build (ALL)
#   make run                 → build + run (target 'run')
#   make clean               → delete build folders
#   make rebuild             → full rebuild
#   make preset=name run     → override preset (ex: dev-msvc)
#   make BUILD_PRESET=name   → override build preset (ex: build-msvc)
#
# 🔧 Git / Release commands:
#   make commit              → Commit all changes on dev
#   make push                → Push dev branch
#   make merge               → Merge dev → main
#   make tag VERSION=vX.Y.Z  → Create + push annotated tag
#   make release VERSION=vX.Y.Z → Full changelog + commit + merge + tag
#   make test                → Run tests (ctest)
# =============================================================

# --------------------------
# CMake Presets
# --------------------------
PRESET       ?= dev-ninja
BUILD_PRESET ?= $(PRESET)

ifeq ($(PRESET),dev-ninja)
  BUILD_PRESET := build-ninja
endif
ifeq ($(PRESET),dev-msvc)
  BUILD_PRESET := build-msvc
endif

RUN_PRESET ?= $(BUILD_PRESET)
ifeq ($(PRESET),dev-ninja)
  RUN_PRESET := run-ninja
endif
ifeq ($(PRESET),dev-msvc)
  RUN_PRESET := run-msvc
endif

CMAKE ?= cmake

# --------------------------
# Git / Release Config
# --------------------------
VERSION     ?= v0.1.0
BRANCH_DEV   = dev
BRANCH_MAIN  = main

.PHONY: help build run clean rebuild preset commit push merge tag release test changelog dev

# --------------------------
# Help
# --------------------------
help:
	@echo ""
	@echo "🧩 Vix App Build System — Softadastra Backend"
	@echo "---------------------------------------------------"
	@echo "🛠️  Build commands:"
	@echo "  make build               - Configure + Build all targets"
	@echo "  make run                 - Build + Run target 'run'"
	@echo "  make clean               - Remove build folders"
	@echo "  make rebuild             - Clean + Rebuild all"
	@echo ""
	@echo "🧬 Release commands:"
	@echo "  make commit              - Add + Commit changes on $(BRANCH_DEV)"
	@echo "  make push                - Push $(BRANCH_DEV)"
	@echo "  make merge               - Merge $(BRANCH_DEV) → $(BRANCH_MAIN)"
	@echo "  make tag VERSION=vX.Y.Z  - Create annotated Git tag"
	@echo "  make release VERSION=vX.Y.Z - Full release chain (changelog + commit + push + merge + tag)"
	@echo ""
	@echo "🧪 Tests:"
	@echo "  make test                - Run CTest"
	@echo ""
	@echo "💡 Quick dev shortcut:"
	@echo "  make dev                 - Build + Run (for fast iteration)"
	@echo ""

# --------------------------
# CMake Build / Run
# --------------------------
all: build

build:
	@echo "🔨 Building using preset '$(PRESET)'..."
	@$(CMAKE) --preset $(PRESET)
	@$(CMAKE) --build --preset $(BUILD_PRESET)

run:
	@echo "🚀 Running app via preset '$(RUN_PRESET)'..."
	@$(CMAKE) --preset $(PRESET)
	@$(CMAKE) --build --preset $(RUN_PRESET) --target run

clean:
	@echo "🧹 Cleaning build folders..."
	rm -rf build-* CMakeFiles CMakeCache.txt

rebuild: clean build

dev: build run

preset:
	@:

# --------------------------
# Git Workflow
# --------------------------
commit:
	git checkout $(BRANCH_DEV)
	@if [ -n "$$(git status --porcelain)" ]; then \
		echo "📝 Committing changes..."; \
		git add .; \
		git commit -m "chore(release): prepare $(VERSION)"; \
	else \
		echo "✅ Nothing to commit."; \
	fi

push:
	@echo "📤 Pushing $(BRANCH_DEV)..."
	git push origin $(BRANCH_DEV)

merge:
	@echo "🔀 Merging $(BRANCH_DEV) → $(BRANCH_MAIN)..."
	git checkout $(BRANCH_MAIN)
	git merge --no-ff --no-edit $(BRANCH_DEV)
	git push origin $(BRANCH_MAIN)
	git checkout $(BRANCH_DEV)

tag:
	@if git rev-parse $(VERSION) >/dev/null 2>&1; then \
		echo "❌ Tag $(VERSION) already exists."; \
		exit 1; \
	else \
		echo "🏷️  Creating tag $(VERSION)..."; \
		git tag -a $(VERSION) -m "Release version $(VERSION)"; \
		git push origin $(VERSION); \
	fi

release:
	@echo "🚢 Releasing version $(VERSION)..."
	make changelog
	make commit
	make push
	make merge
	make tag VERSION=$(VERSION)
	@echo "✅ Release $(VERSION) completed!"

# --------------------------
# Tests & Changelog
# --------------------------
test:
	@echo "🧪 Running tests..."
	@cd build && ctest --output-on-failure || true

changelog:
	@echo "🗒️  Updating CHANGELOG.md..."
	@bash scripts/update_changelog.sh || echo "⚠️  No changelog script found."
