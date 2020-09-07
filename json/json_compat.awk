@load "json"

function json_toJSON(data, do_linear)
{
	return json::to_json(data, do_linear)
}

function json_fromJSON(input_string, output_array)
{
	return json::from_json(input_string, output_array)
}
