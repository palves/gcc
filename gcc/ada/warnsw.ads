------------------------------------------------------------------------------
--                                                                          --
--                         GNAT COMPILER COMPONENTS                         --
--                                                                          --
--                               W A R N S W                                --
--                                                                          --
--                                 S p e c                                  --
--                                                                          --
--          Copyright (C) 1999-2014, Free Software Foundation, Inc.         --
--                                                                          --
-- GNAT is free software;  you can  redistribute it  and/or modify it under --
-- terms of the  GNU General Public License as published  by the Free Soft- --
-- ware  Foundation;  either version 3,  or (at your option) any later ver- --
-- sion.  GNAT is distributed in the hope that it will be useful, but WITH- --
-- OUT ANY WARRANTY;  without even the  implied warranty of MERCHANTABILITY --
-- or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License --
-- for  more details.  You should have  received  a copy of the GNU General --
-- Public License  distributed with GNAT; see file COPYING3.  If not, go to --
-- http://www.gnu.org/licenses for a complete copy of the license.          --
--                                                                          --
-- GNAT was originally developed  by the GNAT team at  New York University. --
-- Extensive contributions were provided by Ada Core Technologies Inc.      --
--                                                                          --
------------------------------------------------------------------------------

--  This unit contains the routines used to handle setting of warning options

package Warnsw is

   -------------------
   -- Warning Flags --
   -------------------

   --  These flags are activated or deactivated by -gnatw switches and control
   --  whether warnings of a given class will be generated or not.

   --  Note: most of these flags are still in opt, but the plan is to move them
   --  here as time goes by. And in fact a really nice idea would be to put
   --  them all in a Warn_Record so that they would be easy to save/restore.

   Warn_On_Record_Holes : Boolean := False;
   --  Warn when explicit record component clauses leave uncovered holes (gaps)
   --  in a record layout. Off by default, set by -gnatw.h (but not -gnatwa).

   Warn_On_Overridden_Size : Boolean := False;
   --  Warn when explicit record component clause or array component_size
   --  clause specifies a size that overrides a size for the typen which was
   --  set with an explicit size clause. Off by default, modified by use of
   --  -gnatw.s/.S, but not set by -gnatwa.

   Warn_On_Standard_Redefinition : Boolean := False;
   --  Warn when a program defines an identifier that matches a name in
   --  Standard. Off by default, modified by use of -gnatw.k/.K, but not
   --  affected by -gnatwa.

   -----------------------------------
   -- Saving and Restoring Warnings --
   -----------------------------------

   --  Type used to save and restore warnings

   type Warning_Record is record
      Address_Clause_Overlay_Warnings     : Boolean;
      Check_Unreferenced                  : Boolean;
      Check_Unreferenced_Formals          : Boolean;
      Check_Withs                         : Boolean;
      Constant_Condition_Warnings         : Boolean;
      Elab_Info_Messages                  : Boolean;
      Elab_Warnings                       : Boolean;
      Implementation_Unit_Warnings        : Boolean;
      Ineffective_Inline_Warnings         : Boolean;
      List_Body_Required_Info             : Boolean;
      List_Inherited_Aspects              : Boolean;
      No_Warn_On_Non_Local_Exception      : Boolean;
      Warning_Doc_Switch                  : Boolean;
      Warn_On_Ada_2005_Compatibility      : Boolean;
      Warn_On_Ada_2012_Compatibility      : Boolean;
      Warn_On_All_Unread_Out_Parameters   : Boolean;
      Warn_On_Assertion_Failure           : Boolean;
      Warn_On_Assumed_Low_Bound           : Boolean;
      Warn_On_Atomic_Synchronization      : Boolean;
      Warn_On_Bad_Fixed_Value             : Boolean;
      Warn_On_Biased_Representation       : Boolean;
      Warn_On_Constant                    : Boolean;
      Warn_On_Deleted_Code                : Boolean;
      Warn_On_Dereference                 : Boolean;
      Warn_On_Export_Import               : Boolean;
      Warn_On_Hiding                      : Boolean;
      Warn_On_Modified_Unread             : Boolean;
      Warn_On_No_Value_Assigned           : Boolean;
      Warn_On_Non_Local_Exception         : Boolean;
      Warn_On_Object_Renames_Function     : Boolean;
      Warn_On_Obsolescent_Feature         : Boolean;
      Warn_On_Overlap                     : Boolean;
      Warn_On_Overridden_Size             : Boolean;
      Warn_On_Parameter_Order             : Boolean;
      Warn_On_Questionable_Missing_Parens : Boolean;
      Warn_On_Record_Holes                : Boolean;
      Warn_On_Redundant_Constructs        : Boolean;
      Warn_On_Reverse_Bit_Order           : Boolean;
      Warn_On_Standard_Redefinition       : Boolean;
      Warn_On_Suspicious_Contract         : Boolean;
      Warn_On_Suspicious_Modulus_Value    : Boolean;
      Warn_On_Unchecked_Conversion        : Boolean;
      Warn_On_Unordered_Enumeration_Type  : Boolean;
      Warn_On_Unrecognized_Pragma         : Boolean;
      Warn_On_Unrepped_Components         : Boolean;
      Warn_On_Warnings_Off                : Boolean;
   end record;

   function Save_Warnings return Warning_Record;
   --  Returns current settingh of warnings

   procedure Restore_Warnings (W : Warning_Record);
   --  Restores current settings of warning flags from W

   -----------------
   -- Subprograms --
   -----------------

   function Set_Warning_Switch (C : Character) return Boolean;
   --  This function sets the warning switch or switches corresponding to the
   --  given character. It is used to process a -gnatw switch on the command
   --  line, or a character in a string literal in pragma Warnings. Returns
   --  True for valid warning character C, False for invalid character.

   function Set_Dot_Warning_Switch (C : Character) return Boolean;
   --  This function sets the warning switch or switches corresponding to the
   --  given character preceded by a dot. Used to process a -gnatw. switch on
   --  the command line or .C in a string literal in pragma Warnings. Returns
   --  True for valid warning character C, False for invalid character.

   procedure Set_GNAT_Mode_Warnings;
   --  This is called in -gnatg mode to set the warnings for gnat mode. It is
   --  also used to set the proper warning statuses for -gnatw.g. Note that
   --  this set of warnings is disjoint from -gnatwa, it enables warnings that
   --  are not included in -gnatwa, and it disables warnings that are included
   --  in -gnatwa (such as Warn_On_Implementation_Units, which we clearly want
   --  to be False for units built with -gnatg).

end Warnsw;
