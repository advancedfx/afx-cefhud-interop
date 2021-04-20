function toPromise(interopFn,...args) {

	if(undefined == interopFn)
		console.log(new Error("interopFn undefined"));

	var self = this;
	return new Promise((resolve,reject) => {
		interopFn.apply(self,[resolve,reject,...args]);
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
