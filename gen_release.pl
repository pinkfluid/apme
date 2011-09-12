#!/usr/bin/env perl
use strict vars;

my $apme_version_h      = "src/version.h";
my $apme_exe            = "src/APme.exe";
my $apme_release_txt    = "RELEASE.txt";

# Figure out the current version
sub apme_version()
{
    my $ret;

    $ret = open(VERSION_H, "<", $apme_version_h);
    if (!$ret)
    {
        die("Unable to open $apme_version_h");
    }

    my $line;
    my ($ver_major, $ver_minor, $ver_rev, $ver_name);

    while ($line = <VERSION_H>)
    {
        if ($line =~ /^\s*#define\s+APME_VERSION_MAJOR\s+(\d+)/)
        {
            $ver_major = $1;
        }
        elsif ($line =~ /^\s*#define\s+APME_VERSION_MINOR\s+(\d+)/)
        {
            $ver_minor = $1;
        } 
        elsif ($line =~ /^\s*#define\s+APME_VERSION_REVISION\s+(\d+)/)
        {
            $ver_rev = $1;
        }
        elsif ($line =~ /^\s*#define\s+APME_VERSION_NAME\s+"(\w+)"/)
        {
            $ver_name = $1;
        }
    }

    close(VERSION_H);

    if (!defined($ver_major) ||
        !defined($ver_minor) ||
        !defined($ver_rev)   ||
        !defined($ver_name))
    {
        die("Unable to parse version.h. A field is missing: MAJOR: $ver_major, MINOR: $ver_minor, REV: $ver_rev, NAME: $ver_name");
    }

    return ($ver_major, $ver_minor, $ver_rev, $ver_name);
}

sub apme_exe_check()
{
    my ($ret, $line);

    # Check if APme.exe exists.
    if (! -e "$apme_exe")
    {
        die("$apme_exe does not exist. Please do a full build before making the release.");
    }

    # Avoid this common mistake 
    $ret = open(LDD_APME_EXE, "-|", "ldd '$apme_exe'");
    if (!$ret)
    {
        die("Unable to run LDD on $apme_exe");
    }

    while ($line = <LDD_APME_EXE>)
    {
        if ($line =~ /cygwin/)
        {
            print("WARNING!!! $apme_exe looks like a Cygwin binary. Continuing in 10 seconds.\n");
#            sleep(10);
        }
    }
    close(LDD_APME_EXE);
}

sub apme_release_notes()
{
    my ($ret, $line);

    $ret = open(RELEASE_TXT, ">", "$apme_release_txt");
    if (!$ret)
    {
        die("Unable to create '$apme_release_txt'");
    }

    my ($ver_min, $ver_maj, $ver_rev, $ver_name) = &apme_version();
    printf(RELEASE_TXT "Release notes for APme %d.%d.%d (%s)\n" .
           "-------------------------------------------------\n\n",
           $ver_min,
           $ver_maj,
           $ver_rev,
           $ver_name);

    # Get a list of revisions
    $ret = open(GIT_LOG, "-|", "git log");
    if (!$ret)
    {
        die("Failed to execute: git log");
    }


    my $date = 0;
    my $have_notes = 0;
    my $trimlines = 0;

    while ($line = <GIT_LOG>)
    {
        chomp($line);

        if ($have_notes)
        {
            if ($line =~ /^commit [a-f0-9]+$/)
            {
                printf(RELEASE_TXT "\n");
                $have_notes = 0;
                next;
            }
            elsif ($line =~ /^\s*$/)
            {
                $trimlines++;
            }
            else
            {
                if ($trimlines gt 0)
                {
                    if ($trimlines lt 1000)
                    {
                        printf(RELEASE_TXT "\n");
                    }
                    $trimlines = 0;
                }
                print(RELEASE_TXT "$line\n");
            }
        }
        else
        {
            if ($line =~ /^Date:\s+(.+)$/)
            {
                $date = $1;
            }
            elsif ($line =~ /^\s+RELEASE NOTES:/)
            {
                print(RELEASE_TXT "$date:\n");
                $have_notes = 1;
                $trimlines = 1000;
            }
        }

    }

    close(RELEASE_TXT);
    close(GIT_LOG);

    system("unix2dos '$apme_release_txt'");
}

sub apme_package()
{
    my ($ver_maj, $ver_min, $ver_rev, $ver_code) = &apme_version();

    my ($zip_file) = sprintf("APme-%d.%d.%d.zip", $ver_maj, $ver_min, $ver_rev);
    system("cp -v $apme_exe .");
    system("zip -m '$zip_file' APme.exe RELEASE.txt");
    system("sha256sum '$zip_file' > '$zip_file.sha256'");
}

&apme_exe_check();

my ($ver_major, $ver_minor, $ver_revision, $ver_name) = &apme_version();
printf("APme version %d.%d.%d (%s)\n", $ver_major, $ver_minor, $ver_revision, $ver_name);

printf("Generating release notes...\n");
&apme_release_notes();

printf("Packaging...\n");
&apme_package();

printf("Done!\n");
