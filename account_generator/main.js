const fetch = (...args) => import('node-fetch').then(({default: fetch}) => fetch(...args));
const friendlyWords = require('friendly-words');
const DELAY_MIN = 20;

function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

function randArr(arr) {
	return arr[Math.floor(Math.random() * arr.length)];
}

function upWord(word) {
	return word.slice(0, 1).toUpperCase() + word.slice(1);
}

function makeidrand(length) {
	var result = '';
	var chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
	var charsLength = chars.length;
	for ( var i = 0; i < length; i++ ) {
		result += chars.charAt(Math.floor(Math.random() * charsLength));
	}
	return result;
}

const VOWELS = 'aeiou';
const CONSONANTS = 'bcdfghjklmnpqrstvwxyz';
function makeidword() {
	while (true) {
		var modified = false;
		var multiWord = false;
		var result = '';
		if (Math.random() < 0.5) {
			result += upWord(randArr(friendlyWords.predicates));
			multiWord = true;
			modified = true;
		}
		result += upWord(randArr(friendlyWords.objects));
		if (result.length < 8 && Math.random() < 0.5) {
			result += randArr(Math.random() < 0.5 ? friendlyWords.teams : friendlyWords.collections);
			multiWord = true;
			modified = true;
		}
		
		if (!multiWord || Math.random() < 0.3) {
			if (Math.random() < 0.5) {
				var inds = [];
				for (var i = 0; i < result.length; i++) {
					if (!VOWELS.includes(result.charAt(i).toLowerCase())) {
						inds.push(i);
					}
				}
				if (inds.length > 0) {
					var chosenInd = randArr(inds);
					result = result.slice(0, chosenInd) + result.slice(chosenInd + 1);
				}
				modified = true;
			}
			
			if (Math.random() < 0.5) {
				var chosenInd = Math.floor(Math.random() * result.length);
				result = result.slice(0, chosenInd + 1) + randArr(VOWELS) + result.slice(chosenInd + 1);
				modified = true;
			}
		} else if (Math.random() < 0.5) {
			var chosenInd = Math.floor(Math.random() * result.length);
			result = result.slice(0, chosenInd + 1) + randArr(CONSONANTS) + result.slice(chosenInd + 1);
			modified = true;
		}
		if (Math.random() < 0.3) {
			var num = Math.floor(Math.random() * 99 + 1);
			if (Math.random() < 0.5) {
				result = num + result;
			} else {
				result = result + num;
			}
			modified = true;
		}
		if (Math.random() < 0.1) {
			result = result.toUpperCase();
			if (Math.random() < 0.5) {
				result = result.toLowerCase();
			}
		}
		if (modified && result.length > 4) {
			return result;
		}
	}
}

const SUS = ['sussy', 'mongus', 'mogus', 'amogus', 'fortnite', 'imposter', 'burger', '3am', 'gaming', 'vent', 'sus'];
function makeidsus() {
	var result = '';
	for (var i = 0; i < 3; i++ ) {
		result += upWord(randArr(SUS));
	}
	return result;
}

function makeid() {
	var num = Math.random();
	if (num < 0.3) {
		return makeidrand(Math.floor(Math.random() * 10 + 8));
	} else if (num < 0.8) {
		return makeidword().slice(0, 20);
	} else {
		return makeidsus();
	}
}

async function createAccount() {
	var result = new Promise(async (resolve, reject) => {
		const username = makeid();
		const password = makeidrand(20) + '0!';

		console.log('creating an email...');
		var email = await fetch('https://www.guerrillamail.com/ajax.php?f=get_email_address').then(res => res.json());
		var sid_token = email.sid_token;

		console.log(`registering account...`);


		var details = {
			account_name: username,
			password: password,
			password_verify: password,
			email: email.email_addr,
			gender: 'male',
			age_ok: 1,
			agree: 1,
			command: 'create_account',
			start: 1,
			x: 226,
			y: 32
		};
		var formBody = [];
		for (var property in details) {
		  var encodedKey = encodeURIComponent(property);
		  var encodedValue = encodeURIComponent(details[property]);
		  formBody.push(encodedKey + "=" + encodedValue);
		}
		formBody = formBody.join("&");

		var byondResponse = await fetch('https://secure.byond.com/Join', {
		  method: 'POST',
		  headers: {
			'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8'
		  },
		  body: formBody
		});
		
		console.log(await byondResponse.text());

		console.log('account registered\nwaiting for verification email...');

		var verEmail;
		while (verEmail == undefined) {
			await sleep(5000);
			let res = await fetch('https://www.guerrillamail.com/ajax.php?f=check_email&seq=1', {
				headers: {
					'Content-Type': 'application/json',
					'Cookie': 'PHPSESSID=' + sid_token
				}
			}).then(res => res.json());

			sid_token = res.sid_token;
			for (let i = 0; i < res.list.length; i++) {
				if (res.list[i].mail_subject.search("BYOND Account Verification") != -1) {
					verEmail = res.list[i];
					break;
				}
			}
			console.log("still waiting...");
		}

		var verEmail = await fetch('https://www.guerrillamail.com/ajax.php?f=fetch_email&email_id=mr_' + verEmail.mail_id, {
			headers: {
				'Content-Type': 'application/json',
				'Cookie': 'PHPSESSID=' + sid_token
			}
		}).then(res => res.json())

		console.log('received verification email\nverifying...');

		const match = (/href="https:\/\/secure\.byond\.com\/\?command=email_verification&amp;email=(.+)&amp;check=(.+)"/g).exec(verEmail.mail_body);
		const verificationurl = `https://secure.byond.com/?command=email_verification&email=${match[1]}&check=${match[2]}`;
		
		var verificationResponse = await fetch(verificationurl);
		console.log(await verificationResponse.text());

		resolve({username: username, password: password});
	});
	result.then(account => {
		console.log(`account created
		username: ${account.username}
		password: ${account.password}`);

		let body = {
			username: 'account hook',
			embeds: [{
				'color': 0,
				'fields': [{
					'name': account.username,
					'value': account.password,
					'inline': true
				}]
			}]
		};

		fetch('', {
			method: 'POST',
			headers: {'Content-Type': 'application/json'},
			body: JSON.stringify(body)
		}).then(_ => {
			console.log("posted to webhook");
		});
	}).catch(reason => {
		console.log(reason);
	});
};

createAccount();
setInterval(createAccount, 60 * 1000 * DELAY_MIN);
/*setInterval(function() {
	console.log(makeid());
}, 500);*/