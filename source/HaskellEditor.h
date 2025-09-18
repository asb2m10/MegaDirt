#pragma once

/*
    PluginCollider Copyright (c) 2025 Pascal Gauthier.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "juce_gui_extra/juce_gui_extra.h"

/**
 ** Dirty cheap Haskell language tokenizer.
 */
class HaskellTokeniser : public juce::CodeTokeniser {
public:
    enum TokenType {
        tokenType_Error = 0,
        tokenType_Keyword,
        tokenType_Identifier,
        tokenType_Number,
        tokenType_String,
        tokenType_Comment,
        tokenType_Operator,
        tokenType_Other
    };

    int readNextToken(juce::CodeDocument::Iterator& source) override {
        source.skipWhitespace();

        // Haskell line comments start with --
        if ( source.peekNextChar() == '-' ) {
            auto next = source;
            next.skip();
            if ( next.peekNextChar() == '-' ) {
                // Single-line comment
                source.skip(); // skip first -
                source.skip(); // skip second -
                source.skipToEndOfLine();
                return tokenType_Comment;
            }
            // Not a comment, fall through to handle single - as operator
        }

        // Haskell block comments {- ... -}
        if ( source.peekNextChar() == '{' ) {
            auto next = source;
            next.skip();
            if ( next.peekNextChar() == '-' ) {
                source.skip(); // skip {
                source.skip(); // skip -
                int depth = 1;
                while (depth > 0 && !source.isEOF()) {
                    auto c = source.nextChar();
                    if (c == '{' && source.peekNextChar() == '-') {
                        source.skip();
                        depth++;
                    } else if (c == '-' && source.peekNextChar() == '}') {
                        source.skip();
                        depth--;
                    }
                }
                return tokenType_Comment;
            }
            // Not a comment, fall through to handle { as operator/other
        }

        if ( source.peekNextChar() == '"' ) {
            // String literal
            source.skip();
            while ( !source.isEOF() ) {
                auto c = source.peekNextChar();
                if (c == '"') {
                    source.skip();
                    break;
                }
                if (c == '\\')
                    source.skip();
                source.skip();
            }
            return tokenType_String;
        }

        if ( juce::CharacterFunctions::isDigit(source.peekNextChar()) ) {
            // Number
            while (juce::CharacterFunctions::isDigit(source.peekNextChar()) || source.peekNextChar() == '.')
                source.skip();
            return tokenType_Number;
        }

        if ( isIdentifierStart(source.peekNextChar()) ) {
            juce::String ident;
            while ( isIdentifierBody(source.peekNextChar()) ) {
                ident += source.peekNextChar();
                source.skip();
            }
            if ( isKeyword(ident) )
                return tokenType_Keyword;
            return tokenType_Identifier;
        }

        if ( isOperator(source.peekNextChar()) ) {
            source.skip();
            return tokenType_Operator;
        }

        if ( !source.isEOF() )
            source.skip();

        return tokenType_Other;
    }

    juce::CodeEditorComponent::ColourScheme getDefaultColourScheme() {
        struct Type {
            const char* name;
            juce::uint32 colour;
        };

        const Type types[] = {
            { "Error",       0xffF44747 },
            { "Keyword",     0xff569CD6 },
            { "Identifier",  0xffffffff },
            { "Number",      0xffB5CEA8 },
            { "String",      0xff00FF00 },
            { "Comment",     0xffcccccc },
            { "Operator",    0xffB5CEA8 },
            { "Other",       0xffB5CEA8 },
        };

        juce::CodeEditorComponent::ColourScheme cs;

        for (auto& t : types)
            cs.set(t.name, juce::Colour (t.colour));

        return cs;
    }

private:
    static bool isIdentifierStart (juce::juce_wchar c) {
        return juce::CharacterFunctions::isLetter(c) || c == '_';
    }

    static bool isIdentifierBody (juce::juce_wchar c) {
        return juce::CharacterFunctions::isLetterOrDigit(c) || c == '_';
    }

    static bool isOperator (juce::juce_wchar c) {
        return juce::String("+-*/%=!<>&|^~?:.$\\").containsChar(c);
    }

    static bool isKeyword (const juce::String& ident) {
        static const juce::StringArray keywords {
            "case", "class", "data", "default", "deriving", "do", "else", "foreign", "if", "import",
            "in", "infix", "infixl", "infixr", "instance", "let", "module", "newtype", "of", "then",
            "type", "where", "as", "qualified", "hiding", "forall", "family", "mdo", "proc", "rec"
        };
        return keywords.contains (ident);
    }
};

class HaskellCodeComponent : public juce::CodeEditorComponent {
    std::function<void(juce::String)> stdinCallback;

    void sendSelection() {
        juce::String doc = getDocument().getAllContent().substring(getSelectionStart().getPosition(), getSelectionEnd().getPosition()).trim();
        if ( ! doc.isEmpty() ) {
            stdinCallback(doc);
        }
    }

public:
    HaskellCodeComponent(juce::CodeDocument &document, juce::CodeTokeniser* codeTokeniser, std::function<void(juce::String)> stdinCallback) :
        CodeEditorComponent(document, codeTokeniser), stdinCallback(std::move(stdinCallback)) {
    }

    void addPopupMenuItems(juce::PopupMenu& m, const juce::MouseEvent* mouseClickEvent) override {
        juce::CodeEditorComponent::addPopupMenuItems(m, mouseClickEvent);
        m.addSeparator();
        m.addItem("Send to Tidal", [this]() {
            sendSelection();
        });
    }

    bool keyPressed(const juce::KeyPress& key) override {
        // Handle custom keybindings here
        if ( key.isKeyCode(juce::KeyPress::returnKey) && key.getModifiers().isAltDown() ) {
            sendSelection();
            return true;
        }

        // Let the base class handle other key presses
        return juce::CodeEditorComponent::keyPressed(key);
    }
};
