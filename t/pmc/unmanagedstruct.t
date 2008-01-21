#! parrot
# Copyright (C) 2006-2007, The Perl Foundation.
# $Id$

=head1 NAME

t/pmc/unmanagedstruct.t - test the UnManagedStruct PMC

=head1 SYNOPSIS

    % prove t/pmc/unmanagedstruct.t

=head1 DESCRIPTION

Tests the UnManagedStruct PMC.

=cut

.sub main :main
    .include 'include/test_more.pir'

    plan(1)

    new P0, 'UnManagedStruct'
    ok(1, 'Instantiated .UnManagedStruct')
.end

# Local Variables:
#   mode: cperl
#   cperl-indent-level: 4
#   fill-column: 100
# End:
# vim: expandtab shiftwidth=4 ft=pir:
