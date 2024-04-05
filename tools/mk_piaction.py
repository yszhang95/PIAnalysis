#!/usr/bin/env python3
import argparse
import os
import subprocess
import datetime

parser = argparse.ArgumentParser(
    description='Generate Template of PIEventAction.')
parser.add_argument('name', metavar="Name", type=str, help="Name of PIEventAction")
parser.add_argument('--type', type=str, dest="actiontype", required=True,
                    help="Type of PIEventAction",
                    choices=['filter', 'producer', 'analyzer'])
parser.add_argument('--dir', type=str, dest='dirpath', required=True,
                    help='Parent direcotry where include and src are hosted')

args = parser.parse_args()


actionnames = {
    "filter": "PIEventFilter",
    "producer": "PIEventProducer",
    "analyzer": "PIEventAnalyzer"
}

rootdir = subprocess.check_output(
    ['git', 'rev-parse', '--show-toplevel']).decode('UTF-8').strip()
topdir = rootdir + "/" + args.dirpath
incdir = topdir + "/include"
srcdir = topdir + "/src"


def generate_dir():
    if not os.path.exists(topdir):
        os.makedirs(topdir)
    if not os.path.exists(incdir):
        os.makedirs(incdir)
    if not os.path.exists(srcdir):
        os.makedirs(srcdir)


def generate_header():
    actionfunctions = {
        "filter": """bool filter(const PIEventData&) override;""",
        "producer": """void produce(PIEventData&) override;
    void fill_dummy(PIEventData&) override;""",
        "analyzer": """void analyze(const PIEventData&) override;"""
    }

    constructors = {
        "filter": "{}(const std::string&, const int)".format(args.name),
        "producer": "{}(const std::string&)".format(args.name),
        "analyzer": "{}(const std::string&)".format(args.name)
    }

    macro_def = "__PI_"
    if args.name[0:2] == "PI":
        macro_def += args.name[2:]
    else:
        macro_def += args.name
    macro_def += "__"

    headercontent = """/***********************************************
*                                              *
* Generated at {timenow}.            *
*                                              *
************************************************/
#ifndef {macro_def}
#define {macro_def}

#include "{actionname}.hpp"

namespace PIAna
{{
  class {name} : public {actionname}
  {{
  public:
    {constructor}
    ~{name}();

    void Begin() override;
    void DoAction(PIEventData&) override;
    void End() override;

  protected:
    {actionfunctions}
    void report() override;

  private:
  }};
}};
#endif
""".format(timenow=datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
           macro_def=macro_def, constructor=constructors[args.actiontype],
           actionname=actionnames[args.actiontype],
           name=args.name,
           actionfunctions=actionfunctions[args.actiontype])
    headerfile = incdir + "/{}.hpp".format(args.name)
    with open(headerfile, 'w') as f:
        f.write(headercontent)


def generate_source():
    actionfunctions = {
        "filter": """bool PIAna::{name}::filter(PIEventData& event)
{{
  // fill out what is necessary.
  // this method is called in PIEventFilter::DoAction().
}}""".format(name=args.name),
        "producer": """void PIAna::{name}::produce(PIEventData& event)
{{
  // fill out what is necessary.
  // this method is called in PIEventProducer::DoAction().
  // When no product can be made, call customized fill_dummy().
}}

void PIAna::{name}::fill_dummy(PIEventData& event)
{{
  // fill dummy values here.
}}""".format(name=args.name),
        "analyzer": """void PIAna::{name}::analyzer(const PIEventData& event)
{{
  // fill out what is necessary here.
  // this method is called in PIEventAnalyzer::DoAction().
}}
        """.format(name=args.name)
    }
    constructors = {
        "filter": """PIAna::{name}::{name}(const std::string& name, const int code)
  : PIEventFilter(name, code)
{{
  // fill out what is necessary
}}
""".format(name=args.name),
        "producer": """PIAna::{name}::{name}(const std::string& name)
  : PIEventProducer(name)
{{
  // fill out what is necessary
}}
""".format(name=args.name),
        "analyzer": """PIAna::{name}::{name}(const std::string& name)
  : PIEventAnalyzer(name)
{{
  // fill out what is necessary
}}
""".format(name=args.name)
    }
    sourcecontent = """/***********************************************
*                                              *
* Generated at {timenow}.            *
*                                              *
************************************************/

#include "PIEventData.hpp"
#include "{name}.hpp"

{constructor}
PIAna::{name}::~{name}()
{{
  // fill out what is necessary
}}

void PIAna::{name}::Begin()
{{
  // always call Begin() of base class first to ensure things are initialized.
  {actionname}::Begin();
  // fill out what is necessary from here.

  // give a summary of parameters at the end.
  if (PIEventAction::verbose_) {{
    report();
  }}
}}

void PIAna::{name}::DoAction(PIEventData& event)
{{
  // Do not modify this function.
  {actionname}::DoAction(event);
}}

void PIAna::{name}::End()
{{
  // fill out what is necessary from here.

  // always call End() of base class at the end.
  {actionname}::End();
}}

{actionfunctions}

void PIAna::{name}::report()
{{
  // fill out what is necessary from here.
}}
""".format(timenow=datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
           actionname=actionnames[args.actiontype],
           name=args.name, constructor=constructors[args.actiontype],
           actionfunctions=actionfunctions[args.actiontype])
    sourcefile = srcdir + "/{}.cpp".format(args.name)
    with open(sourcefile, 'w') as f:
        f.write(sourcecontent)


def generate_files():
    generate_dir()
    generate_header()
    generate_source()


if __name__ == '__main__':
    generate_files()
