CXX=g++
CFLAGS=-Iinclude -std=c++11
LDFLAGS=-pthread

ifdef DIMSIZE
CFLAGS+= -DDIMSIZE=${DIMSIZE}
endif

ifdef THRESHOLD
CFLAGS+= -DTHRESHOLD=${THRESHOLD}
endif

ifdef NUM_THREADS
CFLAGS+= -DNUM_THREADS=${NUM_THREADS}
endif

ifdef WORK_MOD
CFLAGS+= -DWORK_MOD=${WORK_MOD}
endif

ifdef PRINT_AND_VERIFY_OUTPUT
CFLAGS+= -DPRINT_AND_VERIFY_OUTPUT=${PRINT_AND_VERIFY_OUTPUT}
endif

ifdef TACC_PAPI_INC
CFLAGS+= -I$(TACC_PAPI_INC) -DPAPI_ENABLED
LDFLAGS+= -Wl,-rpath,$(TACC_PAPI_LIB) -L$(TACC_PAPI_LIB) -lpapi
endif


ifdef $(BUILD)
BUILD=$(BUILD)
endif

BINS = $(BUILD)/serial_permute_best $(BUILD)/par_rec_mm $(BUILD)/serial_run_all $(BUILD)/central $(BUILD)/steal $(BUILD)/share

all : $(BINS)

$(BUILD)/par_rec_mm.o : src/pt1/par_rec_mm.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@
$(BUILD)/serial_run_all.o : src/pt1/serial_run_all.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@
$(BUILD)/central.o : src/pt2/central.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@
$(BUILD)/steal.o : src/pt2/steal.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@
$(BUILD)/share.o : src/pt2/share.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@
$(BUILD)/serial_permute_best.o: src/pt1/serial_permute_best.cpp 
	$(CXX) -c $(PERMUTE_FOR) $(CFLAGS) $^ -o $@ 

$(BUILD)/% : $(BUILD)/%.o $(BUILD)/serial_mm.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(BUILD)/serial_mm.o: src/common/serial_mm.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD)
	mkdir -p $(BUILD)

