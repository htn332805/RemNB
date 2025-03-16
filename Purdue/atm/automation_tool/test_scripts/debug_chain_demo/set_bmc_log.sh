#!/usr/bin/env python3
from scriptlet_base import ScriptletBase
import random, pexpect, time


class NumberProcessor(ScriptletBase):
    def _execute(self):
        # Generate 100 random numbers
        numbers = [random.randint(1, 1000) for _ in range(100)]

        if self.args.debug:
            self.logger.debug(f"Generated {len(numbers)} numbers")
            self.logger.debug(f"First 5 numbers: {numbers[:5]}")

        # Save to output file
        with open(self.args.output_file, "w") as f:
            f.write("\n".join(map(str, numbers)))

        return {"status": "success", "numbers_generated": len(numbers)}


if __name__ == "__main__":
    processor = NumberProcessor("process_numbers")
    result = processor.run()
