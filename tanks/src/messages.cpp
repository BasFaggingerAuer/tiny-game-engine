/*
Copyright 2012, Bas Fagginger Auer.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "messages.h"

using namespace tanks;

TanksMessageTranslator::TanksMessageTranslator() :
    tiny::net::MessageTranslator()
{
    addMessageType(new msg::Help());
    addMessageType(new msg::Host());
    addMessageType(new msg::Join());
    addMessageType(new msg::Disconnect());
    addMessageType(new msg::AddPlayer());
    addMessageType(new msg::RemovePlayer());
    addMessageType(new msg::WelcomePlayer());
}

TanksMessageTranslator::~TanksMessageTranslator()
{

}

