import asyncio
import logging
import sys
from os import getenv

import re

import aiohttp
from aiogram import Bot, Dispatcher, html
from aiogram.client.default import DefaultBotProperties
from aiogram.enums import ParseMode
from aiogram.filters import CommandStart
from aiogram.types import Message

from bot.important_constants import TELEGRAM_BOT_API_TOKEN, MINSTRALAI_API_KEY, MINSTRALAI_REQUEST_URL

# Bot token can be obtained via https://t.me/BotFather
TOKEN = getenv(TELEGRAM_BOT_API_TOKEN)

# All handlers should be attached to the Router (or Dispatcher)

dp = Dispatcher()


@dp.message(CommandStart())
async def command_start_handler(message: Message) -> None:
    """
    This handler receives messages with `/start` command
    """
    # Most event objects have aliases for API methods that can be called in events' context
    # For example if you want to answer to incoming message you can use `message.answer(...)` alias
    # and the target chat will be passed to :ref:`aiogram.methods.send_message.SendMessage`
    # method automatically or call API method directly via
    # Bot instance: `bot.send_message(chat_id=message.chat.id, ...)`
    await message.answer(f"Hello, {html.bold(message.from_user.full_name)}!")

async def query_mistral_api(prompt: str) -> str:
    headers = {
        "Authorization": f"Bearer {MINSTRALAI_API_KEY}",
        "Content-Type": "application/json",
    }
    payload = {
        "model": "mistral-small-latest",
        "temperature": 0.2,
        "max_tokens": 1000,  # adjust based on needs
        "stream": False,
        "random_seed": 0,
        "messages": [
            {
                "role": "user",
                "content": prompt
            }
        ],
        "response_format": {
            "type": "text"
        },
        "tools": [],
        "tool_choice": "auto",
        "presence_penalty": 0,
        "frequency_penalty": 0,
        "n": 1,
        "safe_prompt": False
    }

    async with aiohttp.ClientSession() as session:
        async with session.post(MINSTRALAI_REQUEST_URL, headers=headers, json=payload) as response:
            if response.status == 200:
                data = await response.json()
                print(data)
                # Check if content is available
                return data["choices"][0]["message"]["content"] or "I'm sorry, but I couldn't process that."
            else:
                logging.error(f"Error {response.status}: {await response.text()}")
                return "There was an error with the AI response."

@dp.message()
async def handle_message(message: Message) -> None:
    user_input = message.text

    try:
        ai_response = await query_mistral_api(user_input)
        if not ai_response.strip():
            ai_response = "The response was empty. Please try again."

        # Escape response content for MarkdownV2
        ai_response = escape_markdown(ai_response)
        await message.answer(ai_response, parse_mode=ParseMode.MARKDOWN_V2)
    except Exception as e:
        logging.error(f"Error handling message: {e}")
        error_message = escape_markdown(str(e))
        await message.reply(error_message, parse_mode=ParseMode.MARKDOWN_V2)

def escape_markdown(text: str) -> str:
    # Escapes all special characters for Telegram MarkdownV2
    escape_chars = r'([!_*\[\]()~`>#+\-=|{}.!])'
    return re.sub(escape_chars, r'\\\1', text)

# @dp.message()
# async def echo_handler(message: Message) -> None:
#     """
#     Handler will forward receive a message back to the sender
#
#     By default, message handler will handle all message types (like a text, photo, sticker etc.)
#     """
#     try:
#         # Send a copy of the received message
#         await message.send_copy(chat_id=message.chat.id)
#     except TypeError:
#         # But not all the types is supported to be copied so need to handle it
#         await message.answer("Nice try!")