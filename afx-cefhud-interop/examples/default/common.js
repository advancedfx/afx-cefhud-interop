function toPromise(interopFn,...args) {
	return new Promise((resolve,reject) => {
		interopFn.apply(this,[resolve,reject,...args]);
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
