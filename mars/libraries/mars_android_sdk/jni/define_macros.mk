upper-case = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))


MODULE_MACROS := $(foreach lib, $(LOCAL_STATIC_LIBRARIES),-DMARS_$(lib))
MODULE_MACROS := $(call upper-case , $(MODULE_MACROS))

LOCAL_CFLAGS += $(MODULE_MACROS)
$(info 'module macros:$(MODULE_MACROS)')

