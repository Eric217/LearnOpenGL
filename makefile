default: 
	cmake -B build -G Xcode || exit 996
	@open build/LearnOpenGL.xcodeproj
