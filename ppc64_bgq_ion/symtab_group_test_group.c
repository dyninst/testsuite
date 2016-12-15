#ifdef __cplusplus
extern "C" {
#endif
#include "../src/mutatee_call_info.h"

extern int test_type_info_mutatee();
extern int test_module_mutatee();
extern int test_lookup_func_mutatee();
extern int test_lookup_var_mutatee();
extern int test_line_info_mutatee();
extern int test_symtab_ser_funcs_mutatee();
extern int test_ser_anno_mutatee();

mutatee_call_info_t mutatee_funcs[] = {
  {"test_type_info", test_type_info_mutatee, GROUPED, "test_type_info"},
  {"test_module", test_module_mutatee, GROUPED, "test_module"},
  {"test_lookup_func", test_lookup_func_mutatee, GROUPED, "test_lookup_func"},
  {"test_lookup_var", test_lookup_var_mutatee, GROUPED, "test_lookup_var"},
  {"test_line_info", test_line_info_mutatee, GROUPED, "test_line_info"},
  {"test_symtab_ser_funcs", test_symtab_ser_funcs_mutatee, GROUPED, "test_symtab_ser_funcs"},
  {"test_ser_anno", test_ser_anno_mutatee, GROUPED, "test_ser_anno"}
};

int max_tests = 7;
int runTest[7];
int passedTest[7];
#ifdef __cplusplus
}
#endif
