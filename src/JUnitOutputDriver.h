//
// Created by bill on 7/6/16.
//

#ifndef DYNINST_TESTSUITE_JUNITOUTPUTDRIVER_H
#define DYNINST_TESTSUITE_JUNITOUTPUTDRIVER_H

#include "StdOutputDriver.h"
#include <sstream>


#include <libxml/parser.h>
#include <libxml/tree.h>

struct RungroupResults
{
    RungroupResults(RunGroup* g) :
            failures(0), skips(0), errors(0), tests(0),
            group_node(xmlNewNode(NULL, BAD_CAST "testsuite"))
    {
    }
    RungroupResults() :
            failures(0), skips(0), errors(0), tests(0),
            group_node(NULL)
    {}

    xmlNodePtr add_test(const char* class_name, const char* test_name, float cpu_usage) {
        xmlNodePtr curTest = xmlNewChild(group_node, NULL, BAD_CAST "testcase", NULL);
        xmlSetProp(curTest, BAD_CAST  "class", BAD_CAST class_name);
        xmlSetProp(curTest, BAD_CAST  "test", BAD_CAST test_name);
        std::stringstream t;
        t << cpu_usage;
        xmlNewProp(curTest, BAD_CAST "time", BAD_CAST t.str().c_str());
        tests++;
        t.str("");
        t << tests;
        xmlSetProp(group_node, BAD_CAST "tests", BAD_CAST t.str().c_str());
        return curTest;
    }
    void add_failure() {
        failures++;
        std::stringstream t;
        t << failures;
        xmlSetProp(group_node, BAD_CAST "failures", BAD_CAST t.str().c_str());
    }
    void add_skip() {
        skips++;
        std::stringstream t;
        t << skips;
        xmlSetProp(group_node, BAD_CAST "skipped", BAD_CAST t.str().c_str());
    }
    void add_error() {
        errors++;
        std::stringstream t;
        t << errors;
        xmlSetProp(group_node, BAD_CAST "errors", BAD_CAST t.str().c_str());
    }

    int failures;
    int skips;
    int errors;
    int tests;
    xmlNodePtr group_node;
};

class JUnitOutputDriver : public StdOutputDriver
{
public:
    TESTLIB_DLL_EXPORT JUnitOutputDriver(void* data);
    TESTLIB_DLL_EXPORT virtual ~JUnitOutputDriver();

    // Informs the output driver that any log messages or results should be
    // associated with the test passed in through the attributes parameter
    virtual void startNewTest(std::map<std::string, std::string> &attributes, TestInfo *test, RunGroup *group);


    // Before calling any of the log* methods or finalizeOutput(), the user
    // must have initialized the test output driver with a call to startNewTest()

    virtual void logResult(test_results_t result, int stage=-1);
    virtual void finalizeOutput();
    virtual void vlog(TestOutputStream stream, const char *fmt, va_list args);

private:
    int group_failures;
    int group_skips;
    int group_errors;
    int group_tests;
    std::map<RunGroup*, RungroupResults> groups;
    std::stringstream failure_log;
    xmlDocPtr results;
    xmlNodePtr root;
    RungroupResults cur_group_results;
    xmlNodePtr cur_test;
};


#endif //DYNINST_TESTSUITE_JUNITOUTPUTDRIVER_H
