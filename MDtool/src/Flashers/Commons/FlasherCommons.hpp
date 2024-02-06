#ifndef FLASHERCOMMONS_HPP
#define FLASHERCOMMONS_HPP

#include <iostream>

class FlasherCommons
{
   public:
	static void progressBar(float progress)
	{
		if (progress > 1.0)
			return;

		int barWidth = 50;

		std::cout << "[";
		int pos = barWidth * progress;
		for (int i = 0; i < barWidth; ++i)
		{
			if (i < pos)
				std::cout << "=";
			else if (i == pos)
				std::cout << ">";
			else
				std::cout << " ";
		}
		std::cout << "] " << int(progress * 100.0) << " %\r";
		std::cout.flush();
	}
};

#endif