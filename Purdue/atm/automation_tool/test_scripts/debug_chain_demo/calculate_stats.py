#!/usr/bin/env python3
from scriptlet_base import ScriptletBase
import statistics


class StatsCalculator(ScriptletBase):
    def _execute(self):
        # Read numbers from input file
        with open(self.args.input_file, "r") as f:
            numbers = [float(line.strip()) for line in f]

        if self.args.debug:
            self.logger.debug(f"Read {len(numbers)} numbers")

        # Calculate statistics
        stats = {
            "mean": statistics.mean(numbers),
            "median": statistics.median(numbers),
            "std_dev": statistics.stdev(numbers),
            "min": min(numbers),
            "max": max(numbers),
        }

        if self.args.debug:
            self.logger.debug(f"Calculated statistics: {stats}")

        # Save to output file
        with open(self.args.output_file, "w") as f:
            for key, value in stats.items():
                f.write(f"{key}: {value}\n")

        return stats


if __name__ == "__main__":
    calculator = StatsCalculator("calculate_stats")
    result = calculator.run()
