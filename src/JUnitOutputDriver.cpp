//
// Created by bill on 7/6/16.
//

#include "JUnitOutputDriver.h"

#if !defined(os_windows_test)
#include <sys/types.h>
#include <unistd.h>
#endif

JUnitOutputDriver::JUnitOutputDriver(void *data) : StdOutputDriver(data),
                                                   group_failures(0),
                                                   group_skips(0),
                                                   group_errors(0),
                                                   group_tests(0) {
    results = xmlNewDoc((const xmlChar *) "1.0");
    root  = xmlNewNode(NULL, (const xmlChar *) "testsuites");
    xmlDocSetRootElement(results, root);
    std::stringstream results_log_name;
    results_log_name << "test_results" << getpid() << ".xml";
    streams[HUMAN] = results_log_name.str();
//   log(HUMAN, "<testsuites>\n");
}

JUnitOutputDriver::~JUnitOutputDriver() {
    bool debug = false;
    xmlSaveFormatFileEnc(debug ? "-" : streams[HUMAN].c_str(), results, "UTF-8", 1);
    xmlFreeDoc(results);
    xmlCleanupParser();
    xmlMemoryDump();
//    log(HUMAN, "</testsuites>\n");
//    FILE* human = getHumanFile();
//    fflush(human);
//    if(human != stdout) fclose(human);
}


std::string modeString(RunGroup* group)
{
    switch(group->createmode)
    {
        case CREATE:
            return "create";
        case USEATTACH:
            return "attach";
        case DISK:
            return "disk";
        default:
            return "unknown mode";
    }
}

std::string makeClassName(RunGroup* group)
{
    std::stringstream classname;
    classname << group->modname;
    classname << ".";
    classname << modeString(group);
    classname << "." << group->compiler << "_" << group->abi;
//    if(group->mutatee  && group->mutatee != "")
//    {
//        classname << "." << group->mutatee;
//    }
//    std::string ret = classname.str();
//    int found = ret.find('.');
//    if(found == std::string::npos) found = 0;
//    found = ret.find('_', found);
//    while(found != std::string::npos)
//    {
//        ret[found] = '.';
//        found = ret.find('_', found + 1);
//    }
    return classname.str();
}


// Informs the output driver that any log messages or results should be
// associated with the test passed in through the attributes parameter
void JUnitOutputDriver::startNewTest(std::map <std::string, std::string> &attributes, TestInfo *test, RunGroup *group) {
    auto found = groups.find(group);
    if(found == groups.end())
    {
        found = groups.insert(std::make_pair(group, RungroupResults(group))).first;
        xmlAddChild(root, found->second.group_node);
    }
    float cpu = test->usage.cpuUsage().tv_sec + (float)test->usage.cpuUsage().tv_usec / 1000000.0;
    cur_test = found->second.add_test(makeClassName(group).c_str(), test->name, cpu);
    cur_group_results = found->second;
    xmlNodePtr props = xmlNewChild(cur_group_results.group_node, NULL, BAD_CAST "properties", NULL);
    for(auto i = attributes.begin();
            i != attributes.end();
            ++i)
    {
        xmlNodePtr p = xmlNewChild(props, NULL, BAD_CAST "property", NULL);
        xmlNewProp(p, BAD_CAST i->first.c_str(), BAD_CAST i->second.c_str());
    }
    failure_log.str("");

//    if (group != last_group) {
//        if(last_group)
//        {
//            std::stringstream suitename;
//            suitename << last_group->modname;
//            if(last_group->mutatee != '\0') suitename << "." << last_group->mutatee;
//            log(HUMAN, "<testsuite name=\"%s\" errors=\"%d\" skipped=\"%d\" tests=\"%d\" failures=\"%d\">\n",
//                suitename.str().c_str(), group_errors, group_skips, group_tests, group_failures);
//            log(HUMAN, group_output.str().c_str());
//            log(HUMAN, "</testsuite>\n");
//            FILE* human = getHumanFile();
//            fflush(human);
//            if(human != stdout) fclose(human);
//        }
//        group_failures = 0;
//        group_skips = 0;
//        group_errors= 0;
//        group_tests = 0;
//        group_output.str() = "";
//    }
//    failure_log.str() = "";
//    StdOutputDriver::startNewTest(attributes, test, group);
}


// Before calling any of the log* methods or finalizeOutput(), the user
// must have initialized the test output driver with a call to startNewTest()



void JUnitOutputDriver::logResult(test_results_t result, int stage)
{


//    group_output << "<testcase classname=\"" << makeClassName(last_group);
//    group_output << "\" name=\"" << last_test->name << "\"";
//
//    if (last_test && last_test->usage.has_data()) {
//        float cpu = last_test->usage.cpuUsage().tv_sec + (float)last_test->usage.cpuUsage().tv_usec / 1000000.0;
//        group_output << " time=\"" << cpu << "\"";
//    }
//    group_tests++;
    switch (result) {
        case PASSED:
//            group_output << "/>\n";
            break;

        case FAILED:
            cur_group_results.add_failure();
            xmlNewChild(cur_test, NULL, BAD_CAST("failure"), BAD_CAST(failure_log.str().c_str()));
//            group_output << ">\n<failure>" << failure_log.str() << "</failure>\n";
//            group_failures++;
//            group_output << "</testcase>";
            break;

        case SKIPPED:
            cur_group_results.add_skip();
            xmlNewChild(cur_test, NULL, BAD_CAST "skipped", NULL);
//            group_skips++;
//            group_output << ">\n<skipped />\n";
//            group_output << "</testcase>";
            break;

        case CRASHED:
            cur_group_results.add_error();
            group_errors++;
            xmlNewChild(cur_test, NULL, BAD_CAST "error", BAD_CAST failure_log.str().c_str());
//            group_output << ">\n<error>Test crashed: " << failure_log.str() << "</error>\n";
//            group_output << "</testcase>";
            break;

        default:
            group_errors++;
            xmlNewChild(cur_test, NULL, BAD_CAST "error", BAD_CAST "Testsuite internal error");
//            group_output << ">\n<error>Testsuite internal error, unknown result</error>\n";
//            group_output << "</testcase>\n";
            break;
            // do nothing
    }

}

void JUnitOutputDriver::vlog(TestOutputStream stream, const char *fmt, va_list args)
{
    if(stream == LOGERR)
    {
        char tmp[256];
        vsnprintf(tmp, 256, fmt, args );
        failure_log << tmp;
    }
    else
    {
        StdOutputDriver::vlog(stream, fmt, args);
    }
}
void JUnitOutputDriver::finalizeOutput()
{
    bool debug = true;
    xmlSaveFormatFileEnc(debug ? "-" : streams[HUMAN].c_str(), results, "UTF-8", 1);
}


