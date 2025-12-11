/*
        ==== The Zelix Programming Language ====
---------------------------------------------------------
  - This file is part of the Fluent Programming Language
    codebase. Fluent is a fast, statically-typed and
    memory-safe programming language that aims to
    match native speeds while staying highly performant.
---------------------------------------------------------
  - Fluent is categorized as free software; you can
    redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.
---------------------------------------------------------
  - Fluent is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public
    License for more details.
---------------------------------------------------------
  - You should have received a copy of the GNU General
    Public License along with Fluent. If not, see
    <https://www.gnu.org/licenses/>.
*/

//
// Created by rodrigo on 7/29/25.
//

#pragma once

#include <ankerl/unordered_dense.h>
#include "celery/misc/ansi.h"
#include "celery/misc/hash.h"
#include "args.h"
#include "celery/string/external.h"
#include "celery/string/string.h"
#include "celery/except/base.h"
#include "value.h"

namespace zelix::cli
{
    class app
    {
        const char *name_ = nullptr;
        const char *desc_ = nullptr;

        ankerl::unordered_dense::map<
            Celery::Str::External,
            value,
            Celery::Misc::Hash
        > commands;

        ankerl::unordered_dense::map<
            Celery::Str::External,
            value,
            Celery::Misc::Hash
        > flags;

        // Aliases (cmd name -> alias)
        ankerl::unordered_dense::map<
            Celery::Str::External,
            Celery::Str::External,
            Celery::Misc::Hash
        > cmd_aliases;

        ankerl::unordered_dense::map<
            Celery::Str::External,
            Celery::Str::External,
            Celery::Misc::Hash
        > flag_aliases;

        // Aliases (alias -> cmd name)
        ankerl::unordered_dense::map<
            Celery::Str::External,
            Celery::Str::External,
            Celery::Misc::Hash
        > cmd_aliases_reverse;

        ankerl::unordered_dense::map<
            Celery::Str::External,
            Celery::Str::External,
            Celery::Misc::Hash
        > flag_aliases_reverse;

        const int argc;
        const char **argv;

        void write_val_info(
            Celery::Str::String &msg,
            const Celery::Str::External &name,
            const value &val,
            const bool flag,
            const int alias_padding = 0
        )
        {
            const auto &desc = val.get_description();
            const auto &alias = flag ? flag_aliases[name] : cmd_aliases[name];

            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(Celery::Misc::Ansi::Bright::Black, 5);
            msg.Write(" ~ ", 3);
            msg.Write(desc.Ptr(), desc.Size());
            msg.Write("\n   ", 4);

            for (size_t i = 0; i < name.Size() + 6 + alias_padding + alias.Size(); ++i)
            {
                msg.Write(' ');
            }

            switch (val.get_type())
            {
                case value::STRING:
                {
                    msg.Write("[type=str, default=", 19);
                    const auto str_val = val.get<Celery::Str::External>();
                    msg.Write(str_val.Ptr(), str_val.Size());
                    break;
                }

                case value::BOOL:
                {
                    msg.Write("[type=bool, default=", 20);
                    if (val.get<bool>())
                    {
                        msg.Write("true", 4);
                    }
                    else
                    {
                        msg.Write("false", 5);
                    }

                    break;
                }

                case value::FLOAT:
                {
                    msg.Write("[type=float, default=", 21);
                    const std::string float_str = std::to_string(val.get<float>());
                    msg.Write(float_str.c_str(), float_str.size());
                    break;
                }

                case value::INTEGER:
                {
                    msg.Write("[type=int, default=", 19);
                    const std::string int_str = std::to_string(val.get<int>());
                    msg.Write(int_str.c_str(), int_str.size());
                    break;
                }
            }

            msg.Write(']');
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write('\n');
        }
    public:
        explicit app(
            const char *name,
            const char *description,
            const int argc,
            const char **argv
        )
            : name_(name), desc_(description), argc(argc), argv(argv)
        {
            if (name == nullptr || description == nullptr)
            {
                throw Celery::Except::Exception("Name and description cannot be null");
            }
        }

        [[nodiscard]] const char *name() const
        {
            return name_;
        }

        [[nodiscard]] const char *description() const
        {
            return desc_;
        }

        template <typename T>
        void command(
            const Celery::Str::External &name,
            const Celery::Str::External &alias,
            const Celery::Str::External &description,
            const T &def
        )
        {
            if (commands.contains(name) || cmd_aliases_reverse.contains(alias))
            {
                throw Celery::Except::Exception("Command already exists");
            }

            cmd_aliases[name] = alias;
            cmd_aliases_reverse[alias] = name;
            commands[name] = value(def, description);
        }

        template <typename T>
        void command(
            const char *name,
            const char *alias,
            const char *description,
            const T &value
        )
        {
            command(
                Celery::Str::External(name, strlen(name)),
                Celery::Str::External(alias, strlen(alias)),
                Celery::Str::External(description, strlen(description)),
                value
            );
        }

        template <typename T>
        void flag(
            const Celery::Str::External &name,
            const Celery::Str::External &alias,
            const Celery::Str::External &description,
            const T &def
        )
        {
            if (flags.contains(name) || flag_aliases_reverse.contains(alias))
            {
                throw Celery::Except::Exception("Flag already exists");
            }

            flag_aliases[name] = alias;
            flag_aliases_reverse[alias] = name;
            flags[name] = value(def, description);
        }

        template <typename T>
        void flag(
            const char *name,
            const char *alias,
            const char *description,
            const T &value
        )
        {
            flag(
                Celery::Str::External(name, strlen(name)),
                Celery::Str::External(alias, strlen(alias)),
                Celery::Str::External(description, strlen(description)),
                value
            );
        }

        args parse()
        {
            args parsed_args(
                commands,
                flags,
                cmd_aliases,
                flag_aliases,
                cmd_aliases_reverse,
                flag_aliases_reverse
            );

            parsed_args.parse(argc, argv);
            return parsed_args;
        }

        template <bool Unicode = true>
        [[nodiscard]] Celery::Str::String help()
        {
            Celery::Str::String msg;
            msg.Write(Celery::Misc::Ansi::Bold::Bright::Yellow, 7);
            msg.Write(name_);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write("\n", 1);
            msg.Write(Celery::Misc::Ansi::Bright::Black, 5);
            msg.Write(desc_);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write("\n\n", 2);

            const size_t bin_len = strlen(argv[0]) - 1;
            if (global_error.error_type != error::UNKNOWN)
            {
                msg.Write(Celery::Misc::Ansi::Bold::Bright::Red, 7);
                msg.Write("Error: ", 7);
                msg.Write(Celery::Misc::Ansi::Reset, 4);
                msg.Write(Celery::Misc::Ansi::Bright::Red, 5);

                switch (global_error.error_type)
                {
                    case error::EXPECTED_VALUE:
                        msg.Write("Expected a value", 16);
                        break;

                    case error::NOT_EXPECTED_VALUE:
                        msg.Write("Unexpected value", 16);
                        break;

                    case error::TYPE_MISMATCH:
                        msg.Write("Type mismatch", 13);
                        break;

                    case error::UNKNOWN_COMMAND:
                        msg.Write("Unknown command", 15);
                        break;

                    case error::UNKNOWN_FLAG:
                        msg.Write("Unknown flag", 12);
                        break;

                    default:
                        break;
                }

                msg.Write("\n  ", 3);
                msg.Write(Celery::Misc::Ansi::Reset, 4);
                msg.Write(Celery::Misc::Ansi::Bright::Black, 5);
                msg.Write("➤ ");
                msg.Write(Celery::Misc::Ansi::Reset, 4);

                const bool error_first = global_error.argv_pos == 0;
                if (error_first)
                {
                    msg.Write(Celery::Misc::Ansi::Bright::Red, 5);
                    msg.Write("\u001B[4m", 4);
                }
                else
                {
                    msg.Write(Celery::Misc::Ansi::Bold::Bright::Blue, 7);
                }

                msg.Write(argv[0], bin_len + 1);

                if (!error_first || argc > 1)
                {
                    msg.Write(Celery::Misc::Ansi::Reset, 4);
                    msg.Write(Celery::Misc::Ansi::Black, 5);
                    msg.Write(" ... ", 5);
                    msg.Write(Celery::Misc::Ansi::Reset, 4);
                }

                if (!error_first)
                {
                    msg.Write(Celery::Misc::Ansi::Bright::Red, 5);
                    msg.Write("\u001B[4m", 4);
                    msg.Write(argv[global_error.argv_pos]);
                    msg.Write(Celery::Misc::Ansi::Reset, 4);
                    msg.Write( "\n         ", 10);

                    for (size_t i = 0; i < bin_len + 1; ++i)
                    {
                        msg.Write(' ');
                    }
                }
                else
                {
                    msg.Write(Celery::Misc::Ansi::Reset, 4);
                    msg.Write("\n    ", 5);
                }

                msg.Write(Celery::Misc::Ansi::Green, 5);
                if constexpr (Unicode)
                {
                    msg.Write("⤷");
                }
                else
                {
                    msg.Write("->");
                }
                msg.Write(" help: ", 7);

                switch (global_error.error_type)
                {
                    case error::EXPECTED_VALUE:
                        msg.Write("add a value after this", 22);
                        break;

                    case error::NOT_EXPECTED_VALUE:
                        msg.Write("remove this", 11);
                        break;

                    case error::TYPE_MISMATCH:
                        msg.Write("change the value to match the expected type", 43);
                        break;

                    case error::UNKNOWN_COMMAND:
                        msg.Write("use --help to see a list of commands", 36);
                        break;

                    case error::UNKNOWN_FLAG:
                        msg.Write("use --help to see a list of flags", 33);
                        break;

                    default:
                        break;
                }

                msg.Write(Celery::Misc::Ansi::Reset, 4);
                msg.Write("\n\n", 2);
            }

            msg.Write(Celery::Misc::Ansi::Bright::Yellow, 5);
            msg.Write(
                "Usage:\n",
                7
            );
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(Celery::Misc::Ansi::Bold::Bright::Blue, 7);

            msg.Write(argv[0], bin_len + 1);
            msg.Write(' ');
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write("\u001B[4m", 4);
            msg.Write(Celery::Misc::Ansi::Bright::Magenta, 5);
            msg.Write("[--flags]", 9);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(' ');
            msg.Write("\u001B[4m", 4);
            msg.Write(Celery::Misc::Ansi::Bright::Green, 5);
            msg.Write("<command>", 9);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(' ');
            msg.Write(Celery::Misc::Ansi::Bright::Yellow, 5);
            msg.Write("\u001B[4m", 4);
            msg.Write("[<args>]", 7);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(' ');
            msg.Write("\u001B[4m", 4);
            msg.Write(Celery::Misc::Ansi::Bright::Magenta, 5);
            msg.Write("[--flags]", 9);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write("\n", 1);

            // msg.Write(ANSI_DIM, 4);
            msg.Write("\x1b[2m", 4);
            msg.Write(Celery::Misc::Ansi::Bright::Black, 5);

            for (size_t i = 0; i < bin_len; ++i)
            {
                msg.Write('-');
            }

            msg.Write('>');
            msg.Write(' ');
            msg.Write(Celery::Misc::Ansi::Reset, 4);

            msg.Write(Celery::Misc::Ansi::Dim::Magenta, 7);
            msg.Write("optional", 8);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(Celery::Misc::Ansi::Dim::Green, 7);
            msg.Write("  required", 10);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(Celery::Misc::Ansi::Dim::Yellow, 7);
            msg.Write("  optional", 10);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write(' ');
            msg.Write(Celery::Misc::Ansi::Dim::Magenta, 7);
            msg.Write("optional", 8);
            msg.Write(Celery::Misc::Ansi::Reset, 4);
            msg.Write("\n\n", 2);

            if (!commands.empty())
            {
                msg.Write(Celery::Misc::Ansi::Yellow, 5);
                msg.Write(
                    "Available commands:\n",
                    20
                );
                msg.Write(Celery::Misc::Ansi::Reset, 4);
            }

            for (const auto &[name, val] : commands)
            {
                msg.Write(Celery::Misc::Ansi::Bright::Black, 5);
                msg.Write("  ", 2);
                if constexpr (Unicode)
                {
                    msg.Write("➤ ");
                }
                else
                {
                    msg.Write("> ", 2);
                }
                msg.Write(Celery::Misc::Ansi::Reset, 4);
                msg.Write(Celery::Misc::Ansi::Cyan, 5);
                msg.Write(name.Ptr(), name.Size());

                // Honor aliases
                const auto &alias = cmd_aliases[name];
                msg.Write(", ", 2);
                msg.Write(alias.Ptr(), alias.Size());

                // Write the value's info
                write_val_info(msg, name, val, false);
            }

            if (!flags.empty())
            {
                msg.Write(Celery::Misc::Ansi::Yellow, 5);
                msg.Write(
                    "Available flags:\n",
                    17
                );
                msg.Write(Celery::Misc::Ansi::Reset, 4);
            }

            for (auto &[name, val] : flags)
            {
                msg.Write("  ", 2);
                msg.Write(Celery::Misc::Ansi::Bright::Blue, 5);
                msg.Write("--", 2);
                msg.Write(name.Ptr(), name.Size());

                // Honor aliases
                const auto &alias = flag_aliases[name];
                msg.Write(", -", 3);
                msg.Write(alias.Ptr(), alias.Size());

                write_val_info(msg, name, val, true, 1);
            }

            return msg;
        }
    };
}