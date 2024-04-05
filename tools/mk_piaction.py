#!/usr/bin/env python3
import argparse
import os
import sys
import subprocess
import datetime

parser = argparse.ArgumentParser(
    description='Generate Template of PIEventAction.')
parser.add_argument('name', metavar="Name", type=str, help="Name of PIEventAction")
parser.add_argument('--type', type=str, dest="actiontype", required=True,
                    help="Type of PIEventAction",
                    choices=['filter', 'producer', 'analyzer'])
parser.add_argument('--dir', type=str, dest='dirpath', required=True,
                    help='Relative path where include and src are hosted')

args = parser.parse_args()

actionnames = {
    "filter": "PIEventFilter",
    "producer": "PIEventProducer",
    "analyzer": "PIEventAnalyzer"
}

rootdir = subprocess.check_output(
    ['git', 'rev-parse', '--show-toplevel']).decode(
        'UTF-8').strip('\n').rstrip('/')
topdir = rootdir + "/" + args.dirpath.rstrip('/')
incdir = topdir + "/include"
srcdir = topdir + "/src"
headerfile = incdir + "/{}.hpp".format(args.name)
sourcefile = srcdir + "/{}.cpp".format(args.name)
relheader = "{dirtop}/include/{name}.hpp".format(
    dirtop=args.dirpath.rstrip("/"), name=args.name)
relsource = "{dirtop}/src/{name}.cpp".format(
    dirtop=args.dirpath.rstrip("/"), name=args.name)
timenow = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')


def commentblock(filename):
    """Generate comment block.
    """
    nspan = 70
    comment = "/" + (nspan-1) * "*" + "\n"
    comment += "*" + (nspan-2) * " " + "*\n"
    comment += "* " + timenow + (nspan-3-len(timenow)) * " " + "*\n"
    comment += "* " + filename + (nspan-3-len(filename)) * " " + "*\n"
    comment += "*" + (nspan-2) * " " + "*\n"
    comment += "*" * (nspan-1) + "/\n"
    return comment


def generate_dir():
    """Generate directory for hosting header and source file.
    """
    if not os.path.exists(topdir):
        os.makedirs(topdir)
    if not os.path.exists(incdir):
        os.makedirs(incdir)
    if not os.path.exists(srcdir):
        os.makedirs(srcdir)
    if os.path.exists(headerfile):
        print("File {} exists!".format(headerfile))
    if os.path.exists(sourcefile):
        print("File {} exists!".format(sourcefile))
    if os.path.exists(headerfile) or os.path.exists(sourcefile):
        sys.exit("Nothing is done. Exit because of existence of files above.")


def generate_header():
    """Generate header file.
    """
    actionfunctions = {
        "filter": """bool filter(const PIEventData&) override;""",
        "producer": """void produce(PIEventData&) override;
    void fill_dummy(PIEventData&) override;""",
        "analyzer": """void analyze(const PIEventData&) override;"""
    }

    constructors = {
        "filter": "{}(const std::string&, const int);".format(args.name),
        "producer": "{}(const std::string&);".format(args.name),
        "analyzer": "{}(const std::string&);".format(args.name)
    }

    macro_def = "__PI_"
    if args.name[0:2] == "PI":
        macro_def += args.name[2:]
    else:
        macro_def += args.name
    macro_def += "__"

    headercontent = """{comment}
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

#if 0
  // remove #if #endif block if private members are desired.
  private:
#endif
  }};
}};
#endif
""".format(comment=commentblock(relheader),
           macro_def=macro_def, constructor=constructors[args.actiontype],
           actionname=actionnames[args.actiontype],
           name=args.name,
           actionfunctions=actionfunctions[args.actiontype])
    if os.path.exists(headerfile):
        sys.exit("{} exists! Did not do anything this time.".format(headerfile))
    with open(headerfile, 'w') as f:
        f.write(headercontent)


def generate_source():
    actionfunctions = {
        "filter": """bool PIAna::{name}::filter(PIEventData& event)
{{
  // this method is called in PIEventFilter::DoAction().
  // fill out what is necessary.
}}""".format(name=args.name),
        "producer": """void PIAna::{name}::produce(PIEventData& event)
{{
  // this method is called in PIEventProducer::DoAction().
  // When no product can be made, call customized fill_dummy().
  // fill out what is necessary.
}}

void PIAna::{name}::fill_dummy(PIEventData& event)
{{
  // fill dummy values here.
}}""".format(name=args.name),
        "analyzer": """void PIAna::{name}::analyze(const PIEventData& event)
{{
  // this method is called in PIEventAnalyzer::DoAction().
  // fill out what is necessary here.
}}
""".format(name=args.name)
    }
    constructors = {
        "filter": """PIAna::{name}::{name}(const std::string& name, const int code)
  : PIEventFilter(name, code)
  //, other member variables
{{
  // fill out what is necessary
}}
""".format(name=args.name),
        "producer": """PIAna::{name}::{name}(const std::string& name)
  : PIEventProducer(name)
  //, other member variables
{{
  // fill out what is necessary
}}
""".format(name=args.name),
        "analyzer": """PIAna::{name}::{name}(const std::string& name)
  : PIEventAnalyzer(name)
  //, other member variables
{{
  // fill out what is necessary
}}
""".format(name=args.name)
    }
    anapart = {
        "filter": "",
        "producer": "",
        "analyzer": """  if (PIEventAction::mgr_->out_file()) {{
    PIEventAnalyzer::outputfile_ = PIEventAction::mgr_->out_file();
  }} else {{
    Fatal("PIAna::{name}::Begin()", "Output file is not properly initialized.");
  }}""".format(name=args.name)
    }
    sourcecontent = """{comment}
#include "TError.h"

#include "PIJobManager.hpp"
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
  // always check if manager is available.
  if (!PIEventAction::mgr_) {{
    Fatal("PIAna::{name}", "PIJobManager is not set.");
  }}
{anapart}
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
""".format(comment=commentblock(relsource),
           actionname=actionnames[args.actiontype],
           name=args.name, constructor=constructors[args.actiontype],
           anapart=anapart[args.actiontype],
           actionfunctions=actionfunctions[args.actiontype])
    if os.path.exists(sourcefile):
        sys.exit("{} exists! Did not do anything this time.".format(sourcefile))
    with open(sourcefile, 'w') as f:
        f.write(sourcecontent)


def generate_files():
    generate_dir()
    generate_header()
    generate_source()


if __name__ == '__main__':
    generate_files()
