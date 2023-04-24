#ifndef UI_COMPONENTS
#define UI_COMPONENTS

#include <raylib.h>
#include <raymath.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const int BUTTON_WIDTH_1(260);
const int BUTTON_HEIGHT_1(60);
const int FONT_SIZE_1(20);
const int FONT_SIZE_2(30);
const int FONT_SIZE_3(60);

Sound tick;

std::string userName = "";

enum UIState {
	InMainMenu = 0,
	InScoreScreen = 1,
	InPauseScreen = 2,
	InGameOverScreen = 3,
	InGame = 4
};

// --------------------------------------------------
//                  UI COMPONENTS
// --------------------------------------------------

struct UIComponent {
    Rectangle bounds;

    bool isHovered = false;

    virtual void Draw() = 0;

    virtual bool HandleHover(Vector2 mousePosition) = 0;

    virtual bool HandleClick(Vector2 clickPosition) = 0;
};

struct UIContainer: public UIComponent {
    std::vector<UIComponent*> children;
    bool transparent;
    Color containerColor;

    void AddChild(UIComponent* child) { children.push_back(child); }

    void ClearChildren() { children.clear(); }

    void Draw() override {
        if (!transparent) {
            DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, containerColor);
        }
        for (size_t i = 0; i < children.size(); i++) {
            children[i]->Draw();
        }
    }

    bool HandleHover(Vector2 mousePosition) override {
        for (size_t i = children.size(); i > 0; --i) {
            if (children[i - 1]->HandleHover(mousePosition)) {
                return true;
            }
        }

        return false;
    }

    bool HandleClick(Vector2 clickPosition) override {
        for (size_t i = children.size(); i > 0; --i) {
            if (children[i - 1]->HandleClick(clickPosition)) {
                return true;
            }
        }

        return false;
    }
};

// --------------------------------------------------
//                BACKGROUND IMAGE
// --------------------------------------------------

struct BackgroundImage : public UIComponent {
    Texture backgroundImageTexture;

    void Draw() override {
        DrawTexture(backgroundImageTexture, bounds.x, bounds.y, WHITE);
    }

    bool HandleHover(Vector2 mousePosition) override { return false; }

    bool HandleClick(Vector2 clickPosition) override { return false; }
};

// --------------------------------------------------
//                      BUTTON
// --------------------------------------------------

struct Button : public UIComponent {
    std::string text;
    bool active;
    
    void (*buttonAction)();

    void Draw() override {
        if (isHovered && active) {
            DrawRectangleRec(bounds, RED);
        } else if (active) {
            DrawRectangleRec(bounds, GRAY);
        } else {
            DrawRectangleRec(bounds, DARKGRAY);
        }

        Vector2 textDimensions = MeasureTextEx(GetFontDefault(), text.c_str(), FONT_SIZE_1, 1);

        int textX = (bounds.x + (bounds.width / 2.1)) - (textDimensions.x / 2);
        int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);

        if (active) {
            DrawText(text.c_str(), textX, textY, FONT_SIZE_1, WHITE);
        } else {
            DrawText(text.c_str(), textX, textY, FONT_SIZE_1, LIGHTGRAY);
        }
    }

    bool HandleHover(Vector2 mousePosition) override {
        isHovered = false;
        if (CheckCollisionPointRec(mousePosition, bounds)) {
            isHovered = true;
            return true;
        } else {
            return false;
        }
    }

    bool HandleClick(Vector2 clickPosition) override {
        if (CheckCollisionPointRec(clickPosition, bounds)) {
            if (active) {
                buttonAction();
                return true;
            } else {
                return false;
            }
        }
        return false;
    }
};

// --------------------------------------------------
//                       LABEL
// --------------------------------------------------

struct Label : public UIComponent {
    std::string text;
    int fontSize;
    Color textColor;
    bool centerAlign = true, leftAlign = false, rightAlign = false;

    void Draw() override {
        Vector2 textDimensions = MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
        int textX, textY;
        if (centerAlign) {
            textX = bounds.x - (textDimensions.x / 2);
            textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
        } else if (leftAlign) {
            textX = bounds.x;
            textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
        } else if (rightAlign) {
            textX = bounds.x - textDimensions.x;
            textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
        }

        DrawText(text.c_str(), textX, textY, fontSize, textColor);
    }

    void setCenterAlign() {
        leftAlign = false;
        rightAlign = false;
        centerAlign = true;
    }

    void setLeftAlign() {
        rightAlign = false;
        centerAlign = false;
        leftAlign = true;
    }

    void setRightAlign() {
        centerAlign = false;
        leftAlign = false;
        rightAlign = true;
    }

    bool HandleHover(Vector2 mousePosition) override { return false; }

    bool HandleClick(Vector2 clickPosition) override { return false; }
};

struct TextField : public UIComponent {
    char text[4];
    int letterCount;
    int fontSize;
    Color textColor;
    bool isMax;

    void Draw() override {
        if (letterCount == 0) {
            text[letterCount] = '_';
        }
        userName = text;
        DrawText(text, bounds.x, bounds.y, fontSize, textColor);
    }

    void AddLetter(char letter) {
        text[letterCount] = toupper(letter);
        if ((letterCount < 2) && (letterCount >= 0)) {
            text[letterCount + 1] = '_';
            text[letterCount + 2] = '\0';
        } else {
            text[letterCount + 1] = '\0';
        }

        if (letterCount >= 2) {
            isMax = true;
        }

        letterCount += 1;
    }

    void RemoveLetter() {
        letterCount--;
        if (letterCount < 0) {
            letterCount = 0;
        }

        text[letterCount] = '_';
        text[letterCount + 1] = '\0';
        isMax = false;
    }

    bool HandleHover(Vector2 mousePosition) override { return false; }

    bool HandleClick(Vector2 clickPosition) override { return false; }
};

struct HPBar : public UIComponent {
    int maxHealth;
    int currentHealth;
    Texture heart_full, heart_half, heart_empty;

    void Draw() override {
        float numHearts = maxHealth / 2;
        for(size_t i = 1; i <= numHearts; i++){
            if(currentHealth >= i * 2) {
                DrawTextureEx(heart_full, {bounds.x + (i * 50), bounds.y}, 0, 3, WHITE);
            } else if (currentHealth == (i * 2) - 1) {
                DrawTextureEx(heart_half, {bounds.x + (i * 50), bounds.y}, 0, 3, WHITE);
            } else if (currentHealth < i * 2) {
                DrawTextureEx(heart_empty, {bounds.x + (i * 50), bounds.y}, 0, 3, WHITE);
            }
        }
    }

    void InitBar(int value) {
        maxHealth = value;
        currentHealth = maxHealth;
    }

    void UpdateHealth(int value) {
        if (currentHealth + value > maxHealth) {
            currentHealth = maxHealth;
        } else if (currentHealth + value <= 0) {
            currentHealth = 0;
        } else {
            currentHealth += value;
        }
    }
    

    bool HandleHover(Vector2 mousePosition) override { return false; }

    bool HandleClick(Vector2 clickPosition) override { return false; }
};

struct UILibrary {
    UIContainer rootContainer;

    void Update() {
        rootContainer.HandleHover(GetMousePosition());

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            rootContainer.HandleClick(GetMousePosition());
        }
    }

    void Draw() { rootContainer.Draw(); }
};


#endif