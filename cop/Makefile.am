#
# Makefile.am for the Enterprise Management module.
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

AM_CPPFLAGS = $(iocore_include_dirs) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/records \
  -I$(top_srcdir)/lib/ts \
  -I$(top_srcdir)/mgmt \
  -I$(top_srcdir)/mgmt/cluster \
  -I$(top_srcdir)/mgmt/api/include

MGMT_DEFS = @MGMT_DEFS@
DEFS += $(MGMT_DEFS)

bin_PROGRAMS = traffic_cop

traffic_cop_SOURCES = TrafficCop.cc
traffic_cop_LDFLAGS = @EXTRA_CXX_LDFLAGS@ @LIBTOOL_LINK_FLAGS@
traffic_cop_LDADD = \
  $(top_builddir)/mgmt/api/remote/libtsmgmt.la \
  $(top_builddir)/lib/ts/libtsutil.la \
  @LIBRESOLV@ @LIBSSL@
