#! /usr/bin/env python

"""
This is a stubbed script to demonstrate the desired input and output format for
pre-challenge image metric evaluation.
"""

import json


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(
        description="Compute metric weights from ground truth for a challenge")
    parser.add_argument("groundtruth", type=str,
        help="path containing ground truth files")

    metricWeights = {
        "Adb1": {
            "title": "Average distance of boundaries (label 1)",
            "weight": -0.5
        },
        "Adb2": {
            "title": "Average distance of boundaries (label 2)",
            "weight": -0.5
        },
        "Hdb1": {
            "title": "Hausdorff distance (label 1)",
            "weight": -1
        },
        "Spec1": {
            "title": "Specificity (label 1)",
            "weight": 1
        },
        "Sens1": {
            "title": "Sensitivity (label 1)",
            "weight": 1
        }
        # etc
    }

    print(json.dumps(metricWeights))
