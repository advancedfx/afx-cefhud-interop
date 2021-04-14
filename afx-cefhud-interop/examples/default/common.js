function toPromise(interopFn,...args) {

	fallbackError = new Error("undefined");

	return new Promise((resolve,reject) => {

		function fallbackReject(error) {
			if(undefined === error)
				reject(fallbackError);
			else
				reject(error);
		}

		try {
			interopFn.apply(this,[resolve,fallbackReject,...args]);
		}
		catch(e) {
			fallbackReject(e);
		}
	});
}

function toSoftError(error) {
	error.soft = true;
	return error;
}

function logError(error) {
	if(undefined !== error && undefined !== error.stack)
		console.error(error.stack);
	else {
		console.error("Error ("+error+") without stack trace, alternate stack trace follows:");
		console.error(new Error().stack);
	}
}