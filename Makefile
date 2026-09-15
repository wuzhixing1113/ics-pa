STUID = 251220081
STUNAME = 伍知行
TOKEN = vP3JBg6w

# DO NOT modify the following code!!!

GITFLAGS = -q --author='tracer-ics2026 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
define git_commit
	-@git add $(NEMU_HOME)/.. -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(echo "> $(1)" && echo $(STUID) $(STUNAME) && uname -a && uptime) | git commit -F - $(GITFLAGS)
	-@sync
endef

_default:
	@echo "Please run 'make' under subprojects."

submit:
	git gc
	STUID=$(STUID) STUNAME=$(STUNAME) bash -c "$$(curl -s https://nasa.nju.edu.cn/icspa26/submit.sh)"

count:
	@find . \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 wc -l | tail -1

.PHONY: default submit
