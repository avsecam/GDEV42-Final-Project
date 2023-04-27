#ifndef UI_HANDLER
#define UI_HANDLER

#include <raylib.h>
#include <raymath.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "uicomponents.hpp"

UIState currentGameState = InMainMenu;

void startGame() { currentGameState = InGame; };
void goToMainMenu() { currentGameState = InMainMenu; };
void goToScoreScreen() { currentGameState = InScoreScreen; };
void goToPauseScreen() { currentGameState = InPauseScreen; };
void goToGameOverScreen() { currentGameState = InGameOverScreen; };
void exitGame() { exit(1); };

int num_of_scores = 0;
int max_score = 0;
int min_score = 0;
int newScore = 0;
int health = 0;

bool scoreUpdate = false;

// --------------------------------------------------
//                  MENU BUILDING
// --------------------------------------------------

void saveScore() {
    std::fstream highScoreFile;
    std::ofstream newHighScoreFile;
    int currentScore;
    bool addedNewScore = false;
    std::vector<std::string> scoreList;

    highScoreFile.open("high_scores.txt");
    std::string line;
    float scoreNumber = 1;

    std::string newScoreLine = std::to_string(newScore) + " " + userName;

    while (getline(highScoreFile, line) && scoreList.size() < 10) {
        int end = line.find(" ");
        std::string score = line.substr(0, end - 0);
        std::string name = line.substr(end, line.length());

        currentScore = stoi(score);

        if (!addedNewScore) {
        if (newScore <= currentScore) {
            scoreList.push_back(line);
        } else {
            scoreList.push_back(newScoreLine);
            if (scoreList.size() >= 10) break;
            scoreList.push_back(line);
            addedNewScore = true;
        }
        } else {
        scoreList.push_back(line);
        }
    }

    if (!addedNewScore && scoreList.size() < 10) {
        scoreList.push_back(newScoreLine);
    }

    highScoreFile.close();

    newHighScoreFile.open("high_scores.txt", std::ofstream::trunc);

    for (size_t i = 0; i < scoreList.size(); i++) {
        newHighScoreFile << scoreList[i] << std::endl;
    }

    newHighScoreFile.close();

    scoreUpdate = true;

    currentGameState = InScoreScreen;
};

struct Menu {
    UILibrary uiLibrary;

    virtual void createUI(float windowWidth, float windowHeight) = 0;

    virtual void Update() = 0;

    virtual void loadBackgroundTexture(Texture tex) = 0;

    virtual void unloadBackgroundTexture() = 0;

    void Draw() { uiLibrary.Draw(); }
};

struct MainMenu : public Menu {
    Button startGameButton;
    Button checkHighScoresButton;
    BackgroundImage startMenuBackground;

    void createUI(float windowWidth, float windowHeight) override {
        uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.transparent = true;

        startMenuBackground.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.AddChild(&startMenuBackground);

        startGameButton.text = "START GAME";
        startGameButton.bounds = {
        windowWidth / 2 - BUTTON_WIDTH_1 / 2,
        windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        startGameButton.buttonAction = startGame;
        startGameButton.active = true;
        uiLibrary.rootContainer.AddChild(&startGameButton);

        checkHighScoresButton.text = "CHECK HIGHSCORES";
        checkHighScoresButton.bounds = {
        windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight / 2 + BUTTON_HEIGHT_1,
        BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        checkHighScoresButton.buttonAction = goToScoreScreen;
        checkHighScoresButton.active = true;
        uiLibrary.rootContainer.AddChild(&checkHighScoresButton);
    }

    void loadBackgroundTexture(Texture tex) override {
        startMenuBackground.backgroundImageTexture = tex;
    }

    void unloadBackgroundTexture() override {
        UnloadTexture(startMenuBackground.backgroundImageTexture);
    }

    void Update() override { uiLibrary.Update(); }
};

struct ScoreScreen : public Menu {
    Label highScoreLabel;
    Label* scoreLabel;
    Label* nameLabel;
    Button returnToMainMenuButton;
    std::fstream highScoreFile;
    int currentScore;

    void createUI(float windowWidth, float windowHeight) override {
        uiLibrary.rootContainer.ClearChildren();

        uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.transparent = false;
        uiLibrary.rootContainer.containerColor = WHITE;

        highScoreLabel.text = "HIGH SCORES";
        highScoreLabel.bounds = {windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
        highScoreLabel.fontSize = FONT_SIZE_3;
        highScoreLabel.setCenterAlign();
        highScoreLabel.textColor = BLACK;

        uiLibrary.rootContainer.AddChild(&highScoreLabel);

        highScoreFile.open("high_scores.txt");
        std::string line;
        float scoreNumber = 1;
        while (getline(highScoreFile, line)) {
        std::cout << line << std::endl;
        int end = line.find(" ");
        std::string score = line.substr(0, end - 0);
        std::string name = line.substr(end, line.length());

        currentScore = stoi(score);

        if (scoreNumber == 1) {
            max_score = currentScore;
        }

        scoreLabel = new Label;
        scoreLabel->text = score;
        scoreLabel->bounds = {
            windowWidth / 2 - 20, (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
            0, FONT_SIZE_2};
        scoreLabel->fontSize = FONT_SIZE_2;
        scoreLabel->setRightAlign();
        scoreLabel->textColor = BLACK;

        nameLabel = new Label;
        nameLabel->text = name;
        nameLabel->bounds = {
            windowWidth / 2 + 10, (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
            0, FONT_SIZE_2};
        nameLabel->fontSize = FONT_SIZE_2;
        nameLabel->setLeftAlign();
        nameLabel->textColor = BLACK;

        uiLibrary.rootContainer.AddChild(scoreLabel);
        uiLibrary.rootContainer.AddChild(nameLabel);

        num_of_scores += 1;
        scoreNumber += 1;
        }

        min_score = currentScore;
        highScoreFile.close();

        returnToMainMenuButton.text = "MAIN MENU";
        returnToMainMenuButton.bounds = {
        windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight - BUTTON_HEIGHT_1 * 2,
        BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        returnToMainMenuButton.buttonAction = goToMainMenu;
        returnToMainMenuButton.active = true;
        uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
    }

    void loadBackgroundTexture(Texture tex) override {}

    void unloadBackgroundTexture() override {}

    void Update() override { uiLibrary.Update(); }
};

struct ScoreScreen2 : public Menu {
    Label highScoreLabel;
    Label* scoreLabel;
    Label* nameLabel;
    Button returnToMainMenuButton;
    std::fstream highScoreFile;
    int currentScore;

    void createUI(float windowWidth, float windowHeight) override {
        uiLibrary.rootContainer.ClearChildren();

        uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.transparent = false;
        uiLibrary.rootContainer.containerColor = WHITE;

        highScoreLabel.text = "HIGH SCORES";
        highScoreLabel.bounds = {windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
        highScoreLabel.fontSize = FONT_SIZE_3;
        highScoreLabel.setCenterAlign();
        highScoreLabel.textColor = BLACK;

        uiLibrary.rootContainer.AddChild(&highScoreLabel);

        highScoreFile.open("high_scores.txt");
        std::string line;
        float scoreNumber = 1;
        while (getline(highScoreFile, line)) {
        std::cout << line << std::endl;
        int end = line.find(" ");
        std::string score = line.substr(0, end - 0);
        std::string name = line.substr(end, line.length());

        currentScore = stoi(score);

        if (scoreNumber == 1) {
            max_score = currentScore;
        }

        scoreLabel = new Label;
        scoreLabel->text = score;
        scoreLabel->bounds = {
            windowWidth / 2 - 20, (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
            0, FONT_SIZE_2};
        scoreLabel->fontSize = FONT_SIZE_2;
        scoreLabel->setRightAlign();
        scoreLabel->textColor = BLACK;

        nameLabel = new Label;
        nameLabel->text = name;
        nameLabel->bounds = {
            windowWidth / 2 + 10, (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
            0, FONT_SIZE_2};
        nameLabel->fontSize = FONT_SIZE_2;
        nameLabel->setLeftAlign();
        nameLabel->textColor = BLACK;

        uiLibrary.rootContainer.AddChild(scoreLabel);
        uiLibrary.rootContainer.AddChild(nameLabel);

        num_of_scores += 1;
        scoreNumber += 1;
        }

        min_score = currentScore;
        highScoreFile.close();

        returnToMainMenuButton.text = "QUIT GAME";
        returnToMainMenuButton.bounds = {
        windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight - BUTTON_HEIGHT_1 * 2,
        BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        returnToMainMenuButton.buttonAction = exitGame;
        returnToMainMenuButton.active = true;
        uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
    }

    void loadBackgroundTexture(Texture tex) override {}

    void unloadBackgroundTexture() override {}

    void Update() override { uiLibrary.Update(); }
};

struct PauseScreen : Menu {
    Label pauseScreenLabel;
    Button returnToMainMenuButton, returnToGameButton;
    void createUI(float windowWidth, float windowHeight) override {
        uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.transparent = true;

        pauseScreenLabel.text = "PAUSE SCREEN";
        pauseScreenLabel.bounds = {
        windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
        pauseScreenLabel.fontSize = FONT_SIZE_3;
        pauseScreenLabel.setCenterAlign();
        pauseScreenLabel.textColor = BLACK;

        uiLibrary.rootContainer.AddChild(&pauseScreenLabel);

        returnToMainMenuButton.text = "MAIN MENU";
        returnToMainMenuButton.bounds = {
        (windowWidth / 2 - BUTTON_WIDTH_1 / 2) - (BUTTON_WIDTH_1 * float(0.75)),
        windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        returnToMainMenuButton.buttonAction = goToMainMenu;
        returnToMainMenuButton.active = true;
        uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);

        returnToGameButton.text = "BACK TO GAME";
        returnToGameButton.bounds = {
        (windowWidth / 2 - BUTTON_WIDTH_1 / 2) + (BUTTON_WIDTH_1 * float(0.75)),
        windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        returnToGameButton.buttonAction = startGame;
        returnToGameButton.active = true;
        uiLibrary.rootContainer.AddChild(&returnToGameButton);
    }

    void loadBackgroundTexture(Texture tex) override {}

    void unloadBackgroundTexture() override {}

    void Update() override { uiLibrary.Update(); }
};

struct GameOverScreen : Menu {
    Label gameOverScreenLabel, scoreLabel, setNameLabel;
    Button returnToMainMenuButton, saveScoreButton;
    TextField playerName;
    BackgroundImage gameOverBackground;

    void createUI(float windowWidth, float windowHeight) override {
        uiLibrary.rootContainer.ClearChildren();

        uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.transparent = true;

        gameOverBackground.bounds = {0, 0, windowWidth, windowHeight};
        uiLibrary.rootContainer.AddChild(&gameOverBackground);

        gameOverScreenLabel.text = "GAME OVER";
        gameOverScreenLabel.bounds = {
        windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
        gameOverScreenLabel.fontSize = FONT_SIZE_3;
        gameOverScreenLabel.setCenterAlign();
        gameOverScreenLabel.textColor = WHITE;

        uiLibrary.rootContainer.AddChild(&gameOverScreenLabel);

        scoreLabel.text = "SCORE: 0";
        scoreLabel.bounds = {
        windowWidth / 2, windowHeight / 2 - FONT_SIZE_3 * float(2.5), 0,
        FONT_SIZE_2};
        scoreLabel.fontSize = FONT_SIZE_2;
        scoreLabel.setCenterAlign();
        scoreLabel.textColor = WHITE;

        uiLibrary.rootContainer.AddChild(&scoreLabel);

        setNameLabel.text = "NAME:";
        setNameLabel.bounds = {
        windowWidth / 2 - 20, windowHeight / 2 - FONT_SIZE_3 * float(1.5), 0,
        FONT_SIZE_2};
        setNameLabel.fontSize = FONT_SIZE_2;
        setNameLabel.setRightAlign();
        setNameLabel.textColor = WHITE;

        uiLibrary.rootContainer.AddChild(&setNameLabel);

        playerName.bounds = {
        windowWidth / 2 + 20, windowHeight / 2 - FONT_SIZE_3 * float(1.5), 0,
        FONT_SIZE_2};
        playerName.fontSize = FONT_SIZE_2;
        playerName.textColor = WHITE;
        playerName.letterCount = 0;
        playerName.isMax = false;

        uiLibrary.rootContainer.AddChild(&playerName);

        saveScoreButton.text = "SAVE SCORE";
        saveScoreButton.bounds = {
        windowWidth / 2 - BUTTON_WIDTH_1 / 2,
        windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        saveScoreButton.buttonAction = saveScore;
        saveScoreButton.active = false;
        uiLibrary.rootContainer.AddChild(&saveScoreButton);

        returnToMainMenuButton.text = "QUIT :(";
        returnToMainMenuButton.bounds = {
        windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight / 2 + BUTTON_HEIGHT_1,
        BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
        returnToMainMenuButton.buttonAction = exitGame;
        returnToMainMenuButton.active = true;
        uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
    }

    void loadBackgroundTexture(Texture tex) override {
        gameOverBackground.backgroundImageTexture = tex;
    }

    void unloadBackgroundTexture() override {}

    void Update() {
        uiLibrary.Update();

        if (playerName.isMax) {
        saveScoreButton.active = true;
        } else {
        saveScoreButton.active = false;
        }

        int key = GetCharPressed();
        if ((key >= 32) && (key <= 125) && (playerName.letterCount < 3)) {
        playerName.AddLetter((char)key);
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
        playerName.RemoveLetter();
        }
    }
};


struct HPAndScoreGUI : Menu {
        Label healthLabel, scoreLabel, scoreOutput;
        HPBar hpBar;
        
        void createUI(float windowWidth, float windowHeight) override {
            uiLibrary.rootContainer.bounds = {0, 0, windowWidth, 100};
            uiLibrary.rootContainer.transparent = false;
            uiLibrary.rootContainer.containerColor = BLACK;

            healthLabel.text = "HP:";
            healthLabel.bounds = {
            100, 25, 0, FONT_SIZE_3};
            healthLabel.fontSize = FONT_SIZE_3;
            healthLabel.setLeftAlign();
            healthLabel.textColor = WHITE;

            uiLibrary.rootContainer.AddChild(&healthLabel);

            hpBar.bounds = {
            200, 25, 500, FONT_SIZE_3};
            uiLibrary.rootContainer.AddChild(&hpBar);

            scoreLabel.text = "SCORE:";
            scoreLabel.bounds = {
            800, 25, 0, FONT_SIZE_3};
            scoreLabel.fontSize = FONT_SIZE_3;
            scoreLabel.setLeftAlign();
            scoreLabel.textColor = WHITE;
            uiLibrary.rootContainer.AddChild(&scoreLabel);

            scoreOutput.text = "0";
            scoreOutput.bounds = {
            1050, 25, 0, FONT_SIZE_3};
            scoreOutput.fontSize = FONT_SIZE_3;
            scoreOutput.setLeftAlign();
            scoreOutput.textColor = WHITE;
            uiLibrary.rootContainer.AddChild(&scoreOutput);
        }

        void loadBackgroundTexture(Texture tex) override {}

        void unloadBackgroundTexture() override {}

        void Update() override { 
            uiLibrary.Update(); 
            scoreOutput.text = std::to_string(newScore);
        }
};


struct MenuHandler {
    std::vector<Menu*> menuList;
    MainMenu mainMenu;
    ScoreScreen scoreScreen;
    PauseScreen pauseScreen;
    GameOverScreen gameOverScreen;
    HPAndScoreGUI inGameGUI;
    ScoreScreen2 scoreScreen2;
    float menuWindowWidth, menuWindowHeight;

    void initialize(float windowWidth, float windowHeight) {
        menuWindowWidth = windowWidth;
        menuWindowHeight = windowHeight;

        mainMenu.createUI(windowWidth, windowHeight);
        scoreScreen.createUI(windowWidth, windowHeight);
        pauseScreen.createUI(windowWidth, windowHeight);
        gameOverScreen.createUI(windowWidth, windowHeight);
        inGameGUI.createUI(windowWidth, windowHeight);
        scoreScreen2.createUI(windowWidth, windowHeight);

        currentGameState = InMainMenu;

        menuList.push_back(&mainMenu);
        menuList.push_back(&scoreScreen);
        menuList.push_back(&pauseScreen);
        menuList.push_back(&gameOverScreen);
        menuList.push_back(&inGameGUI);
        menuList.push_back(&scoreScreen2);
    }

    void Update() {
        //if (currentGameState == InGame) return;
        if (scoreUpdate) {
            scoreScreen.createUI(menuWindowWidth, menuWindowHeight);
            scoreUpdate = false;
        }

        menuList[currentGameState]->Update();
    }

    void Draw() {
        //if (currentGameState == InGame) return;
        menuList[currentGameState]->Draw();
    }

    void setState(UIState s) { currentGameState = s; }

    UIState getState() { return currentGameState; }
};

#endif