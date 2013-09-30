#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

#include "c.h"
#include "../pxfuriparser.c"


/*
 * Test parsing of valid uri as given in LOCATION in a PXF external table.
 */
void 
test__parseGPHDUri__ValidURI(void **state)
{
	char* uri = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter&ACCESSOR=AvroFileAccessor&RESOLVER=AvroResolver&ANALYZER=HdfsAnalyzer";
	List* options = NIL;
	ListCell* cell = NULL;
	OptionData* option = NULL;

	GPHDUri* parsed = parseGPHDUri(uri);

	assert_true(parsed != NULL);
	assert_string_equal(parsed->uri, uri);

	assert_string_equal(parsed->protocol, "pxf");
	assert_string_equal(parsed->host, "1.2.3.4");
	assert_string_equal(parsed->port, "5678");
	assert_string_equal(parsed->data, "some/path/and/table.tbl");

	options = parsed->options;
	assert_int_equal(list_length(options), 4);

	cell = list_nth_cell(options, 0);
	option = lfirst(cell);
	assert_string_equal(option->key, "FRAGMENTER");
	assert_string_equal(option->value, "HdfsDataFragmenter");

	cell = list_nth_cell(options, 1);
	option = lfirst(cell);
	assert_string_equal(option->key, "ACCESSOR");
	assert_string_equal(option->value, "AvroFileAccessor");

	cell = list_nth_cell(options, 2);
	option = lfirst(cell);
	assert_string_equal(option->key, "RESOLVER");
	assert_string_equal(option->value, "AvroResolver");

	cell = list_nth_cell(options, 3);
	option = lfirst(cell);
	assert_string_equal(option->key, "ANALYZER");
	assert_string_equal(option->value, "HdfsAnalyzer");

	assert_true(parsed->fragments == NULL);

	freeGPHDUri(parsed);
}

/*
 * Negative test: parsing of uri without protocol delimiter "://"
 */
void
test__parseGPHDUri__NegativeTestNoProtocol(void **state)
{
	char* uri_no_protocol = "pxf:/1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_no_protocol);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf:/1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with a missing equal
 */
void
test__parseGPHDUri__NegativeTestMissingEqual(void **state)
{
	char* uri_missing_equal = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_missing_equal);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER: option 'FRAGMENTER' missing '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with duplicate equals
 */
void
test__parseGPHDUri__NegativeTestDuplicateEquals(void **state)
{
	char* uri_duplicate_equals = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter=DuplicateFragmenter";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_duplicate_equals);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=HdfsDataFragmenter=DuplicateFragmenter: option 'FRAGMENTER=HdfsDataFragmenter=DuplicateFragmenter' contains duplicate '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with a missing key
 */
void
test__parseGPHDUri__NegativeTestMissingKey(void **state)
{
	char* uri_missing_key = "pxf://1.2.3.4:5678/some/path/and/table.tbl?=HdfsDataFragmenter";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_missing_key);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?=HdfsDataFragmenter: option '=HdfsDataFragmenter' missing key before '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

/*
 * Negative test: parsing of a uri with a missing value
 */
void
test__parseGPHDUri__NegativeTestMissingValue(void **state)
{
	char* uri_missing_value = "pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=";

	/* Setting the test -- code omitted -- */
	PG_TRY();
	{
		/* This will throw a ereport(ERROR).*/
		GPHDUri* parsed = parseGPHDUri(uri_missing_value);
	}
	PG_CATCH();
	{
		CurrentMemoryContext = 1;
		ErrorData *edata = CopyErrorData();

		/*Validate the type of expected error */
		assert_true(edata->sqlerrcode == ERRCODE_SYNTAX_ERROR);
		assert_true(edata->elevel == ERROR);
		assert_string_equal(edata->message, "Invalid URI pxf://1.2.3.4:5678/some/path/and/table.tbl?FRAGMENTER=: option 'FRAGMENTER=' missing value after '='");
		return;
	}
	PG_END_TRY();

	assert_true(false);
}

int 
main(int argc, char* argv[]) 
{
	cmockery_parse_arguments(argc, argv);

	const UnitTest tests[] = {
			unit_test(test__parseGPHDUri__ValidURI),
			unit_test(test__parseGPHDUri__NegativeTestNoProtocol),
			unit_test(test__parseGPHDUri__NegativeTestMissingEqual),
			unit_test(test__parseGPHDUri__NegativeTestDuplicateEquals),
			unit_test(test__parseGPHDUri__NegativeTestMissingKey),
			unit_test(test__parseGPHDUri__NegativeTestMissingValue)
	};
	return run_tests(tests);
}
