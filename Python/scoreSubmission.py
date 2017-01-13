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
        truth,
        test
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
    # Iterate over each file and call scoring executable on the pair
    scores = []
    for gt in os.listdir(args.groundtruth):
        sub = matchInputFile(gt, args.submission)
        truth = os.path.join(args.groundtruth, gt)

        scores.append({
            'dataset': gt,
            'metrics': runScoring(truth, sub)
        })

    print(json.dumps(scores))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Submission scoring helper script')
    parser.add_argument('-g', '--groundtruth', required=True,
                        help='path to the ground truth folder')
    parser.add_argument('-s', '--submission', required=True,
                        help='path to the submission folder')
    args = parser.parse_args()

    scoreAll(args)
