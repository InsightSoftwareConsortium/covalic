#!/usr/bin/env python

# This script is used to score a set of submission files against a
# corresponding set of ground truth files. It is the entry point for this
# repository's docker container.

# It matches each input file with each ground truth file, scores each one,
# then builds a JSON response that it prints out at the end representing the
# final scoring output of the whole submission.

from __future__ import print_function

import argparse
import json
import os
import subprocess
import sys
import zipfile


def extractZip(path, dest, flatten=True):
    """
    Extract a zip file, optionally flattening it into a single directory.
    """
    try:
        os.makedirs(dest)
    except OSError:
        if not os.path.exists(dest):
            raise

    with zipfile.ZipFile(path) as zf:
        if flatten:
            for name in zf.namelist():
                out = os.path.join(dest, os.path.basename(name))
                with open(out, 'wb') as ofh:
                    with zf.open(name) as ifh:
                        while True:
                            buf = ifh.read(65536)
                            if buf:
                                ofh.write(buf)
                            else:
                                break
        else:
            zf.extractall(output)


def matchInputFile(gt, subDir):
    """
    Given a ground truth file and an input directory, find a matching input
    file in the input directory (i.e. one with the same prefix but different
    extension). If none exists, raises an exception.
    """
    prefix = gt.split('.')[0]

    for sub in os.listdir(subDir):
        if sub.split('.')[0] == prefix:
            return os.path.join(subDir, sub)

    raise Exception('No matching input file for prefix: ' + prefix)


def runScoring(truth, test):
    """
    Run the scoring executable. This is assumed to be running inside the
    docker container for this repository.
    """
    command = (
        '/covalic/_build/Covalic-Build/Code/Testing/validateLabelImages',
        '/data/groundtruth/' + os.path.basename(truth),
        '/data/submission/' + os.path.basename(test)
    )

    p = subprocess.Popen(args=command, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    if p.returncode != 0:
        print('Error scoring %s:' % test, file=sys.stderr)
        print('Command: ' + ' '.join(command), file=sys.stderr)
        print('STDOUT: ' + stdout, file=sys.stderr)
        print('STDERR: ' + stderr, file=sys.stderr)

        raise Exception('Scoring subprocess returned error code {}'.format(
            p.returncode))

    metrics = []
    for line in stdout.splitlines():
        name, value = line.split('=')
        metrics.append({
            'name': name,
            'value': value
        })

    return metrics


def scoreAll(args):
    localDirs = {}

    # Unzip the input files into appropriate folders
    dest = os.path.dirname(args.groundtruth)
    gtDir = os.path.join(dest, 'groundtruth')
    subDir = os.path.join(dest, 'submission')
    extractZip(args.groundtruth, gtDir)
    extractZip(args.submission, subDir)

    # Iterate over each file and call scoring executable on the pair
    scores = []
    for gt in os.listdir(gtDir):
        sub = matchInputFile(gt, subDir)
        truth = os.path.join(gtDir, gt)

        scores.append({
            'dataset': gt,
            'metrics': runScoring(truth, sub)
        })

    print(json.dumps(scores))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Submission scoring helper script')
    parser.add_argument('-g', '--groundtruth', required=True,
                        help='path to the ground truth zip file')
    parser.add_argument('-s', '--submission', required=True,
                        help='path to the submission zip file')
    args = parser.parse_args()

    scoreAll(args)
