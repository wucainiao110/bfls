BFLSDIR:=$(CURDIR)
INCPATH:=-I$(BFLSDIR)/include -I./
SRCPATH:=$(BFLSDIR)/src
LIBPATH:=$(BFLSDIR)/lib
 
CC=gcc
AR=ar
 
# ����libĿ¼���������Ŀ¼����Ҫ��Ӧ����xxx_subdir������util_subdir
LIBNAME:=util
 
#���ֺ�������
 
#��չԴĿ¼
define EXTEND_DIR
  ifeq ($($(1)_subdir),)
    DIR_$(1)=$(SRCPATH)/$(1)
  else
    DIR_$(1)=$(SRCPATH)/$(1) $(foreach n,$($(1)_subdir),$(SRCPATH)/$(1)/$(n))
  endif

  ifneq ($(LIBPATH)/$(1),$(wildcard $(LIBPATH)/$(1)))
    ODIR_$(1)=$(LIBPATH)/$(1)
  endif
endef
 
# .o���� $(1)��lib���ƣ� $(2)��.c����
# ��֪��Ϊ�Σ�-MT�ǣ�$(subst .d,.o,$$@)û��    
define BUILD_DEP
$(LIBPATH)/$(1)/$(subst .c,.d,$(notdir $(2))):$(2) $(ODIR_$(1))
	$(CC) -MM -MT $(LIBPATH)/$(1)/$(subst .c,.o,$(notdir $2)) $(INCPATH) -I$(dir $(2)) $$< >$$@
endef
 
define BUILD_OBJ
$(LIBPATH)/$(1)/$(subst .c,.o,$(notdir $(2))):$(2)
	$(CC) -c -O2 $$< $(INCPATH) -I$(dir $(2)) -o $$@
endef
 
define BUILD_LIB
$(LIBPATH)/lib$(1).a:$(OBJ_$(1))
	$(AR) crus $$@ $$^
endef
 
OBJPATH=$(foreach n,$(LIBNAME),$(LIBPATH)/$(n))
LIBS=$(foreach n,$(LIBNAME),$(LIBPATH)/lib$(n).a)
 
# DIR_XXXX ÿ��lib������Դ�ļ�·��
$(foreach n,$(LIBNAME),$(eval $(call EXTEND_DIR,$(n))))
 
# SRC_XXXX
$(foreach n,$(LIBNAME),$(eval SRC_$(n)=$(foreach d,$(DIR_$(n)),$(wildcard $(d)/*.c))))
 
# OBJ_XXXX
$(foreach n,$(LIBNAME),$(eval OBJ_$(n)=$(foreach s,$(SRC_$(n)),$(LIBPATH)/$(n)/$(subst .c,.o,$(notdir $(s))))))
 
# DEP_XXXX
#$(foreach n,$(LIBNAME),$(eval $(call EXTEND_DEP,$(n))))
$(foreach n,$(LIBNAME),$(eval DEP_$(n)=$(subst .o,.d,$(OBJ_$(n)))))
 
 
lib:$(OBJPATH) $(LIBS) 

test: $(SRCPATH)/test.c lib
	$(CC) -o $@.exe $< -L$(LIBPATH) $(foreach n,$(LIBNAME),-l$(n))

clean:
	-rm -rf $(LIBPATH)/*

# ����objĿ¼
dirs:$(OBJPATH)
$(OBJPATH):
	mkdir $@
 
# include depend
ifneq ($(MAKECMDGOALS),clean)
$(foreach n,$(LIBNAME),$(eval -include $(DEP_$(n))))
endif
 
# depend build rule
$(foreach n,$(LIBNAME),$(foreach s,$(SRC_$(n)),$(eval $(call BUILD_DEP,$(n),$(s)))))
 
# obj build rule
$(foreach n,$(LIBNAME),$(foreach s,$(SRC_$(n)),$(eval $(call BUILD_OBJ,$(n),$(s)))))
 
# lib build rule
$(foreach n,$(LIBNAME),$(eval $(call BUILD_LIB,$(n),$(s))))
