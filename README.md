<br>
<a href="https://imstyles.dannybanno.repl.co">
  <h1 align="center">! Imgui Styles For Sale !</h1>
  <h1 align="center">! imstyles.dannybanno.repl.co !</h1>
</a>
<br>
<br>

<h1>Particle background for imgui</h1>
<h3>always liked the look of these sort of background so thought id have a go at making one and think it has came out pretty good</h3>
<img src="https://i.imgur.com/PtNmjH9.png">

## Usage
Initializes the particles setting the x and y to random and sets the velocity which will be used later to set the position  
```c++
  static bool initialized = false;
	if (!initialized)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			particlePositions[i] = ImVec2(
				ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * static_cast<float>(rand()) / RAND_MAX,
				ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * static_cast<float>(rand()) / RAND_MAX
			);

			particleVelocities[i] = ImVec2(
				static_cast<float>((rand() % 11) - 5),
				static_cast<float>((rand() % 11) - 5)
			);

		}

		initialized = true;
	}
```
Draws the lines to the cursor and particles depending on the distance 
```c++
  ImVec2 cursorPos = ImGui::GetIO().MousePos;
	for (int i = 0; i < numParticles; ++i)
	{
		//draw lines to particles
		for (int j = i + 1; j < numParticles; ++j)
		{
			float distance = std::hypotf(particlePositions[j].x - particlePositions[i].x, particlePositions[j].y - particlePositions[i].y);
			float opacity = 1.0f - (distance / 55.0f);  // opacity cahnge

			if (opacity > 0.0f)
			{
				ImU32 lineColor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacity));
				drawList->AddLine(particlePositions[i], particlePositions[j], lineColor);
			}
		}

		//draw lines to cursor
		float distanceToCursor = std::hypotf(cursorPos.x - particlePositions[i].x, cursorPos.y - particlePositions[i].y);
		float opacityToCursor = 1.0f - (distanceToCursor / 52.0f);  // Adjust the divisor to control the opacity change

		if (opacityToCursor > 0.0f)
		{
			ImU32 lineColorToCursor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacityToCursor));
			drawList->AddLine(cursorPos, particlePositions[i], lineColorToCursor);
		}
	}
```
Updates adn renders the particles setting the positions, keeping them in bound of the window and setting the colour
```c++
float deltaTime = ImGui::GetIO().DeltaTime;
	for (int i = 0; i < numParticles; ++i)
	{
		particlePositions[i].x += particleVelocities[i].x * deltaTime;
		particlePositions[i].y += particleVelocities[i].y * deltaTime;

		// Stay in window
		if (particlePositions[i].x < ImGui::GetWindowPos().x)
			particlePositions[i].x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
		else if (particlePositions[i].x > ImGui::GetWindowPos().x + ImGui::GetWindowSize().x)
			particlePositions[i].x = ImGui::GetWindowPos().x;

		if (particlePositions[i].y < ImGui::GetWindowPos().y)
			particlePositions[i].y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
		else if (particlePositions[i].y > ImGui::GetWindowPos().y + ImGui::GetWindowSize().y)
			particlePositions[i].y = ImGui::GetWindowPos().y;

		ImU32 particleColour = ImGui::ColorConvertFloat4ToU32(settings::particleColour);

		//render particles behind components
		drawList->AddCircleFilled(particlePositions[i], 1.5f, particleColour);
	}
```

<h6>credits to cazzwastaken for the base of the imgui: https://github.com/cazzwastaken/borderless-imgui-window</h6>
