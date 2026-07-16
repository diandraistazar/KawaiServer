<!DOCTYPE html>

<html>
<head>
	<title>Diandra's webserver</title>
	<style>
		:root {
			font-size: 14px;
		}

		.header {
			text-align: center;
		}
		.header #title h1 #directory {
			color: blue;
		}
		.header #title h1 #admin {
			color: red;
			letter-spacing: -2px;
		}
		.header p {
			font-size: 1rem;
		}
		.header * {
			margin: auto;
		}

		.main {
			list-style: none;
		}
		.main li {
			margin: 0 0 0 -2.5rem;
		}
	</style>
</head>
<body>
	<?php
	
	$root_path = getenv("ROOT_DIR");;
	$req_path = getenv("REQUEST_PATH");
	$full_path = "$root_path/$req_path";

	$user = getenv("USER");
	$hostname = gethostname();
	
	$list_dir = scandir($full_path);
	if($list_dir != false)
		$list = array_diff($list_dir, array(".", ".."));
	else
		$list = [];

	echo "<header class='header'> 
			<div id='title'>
				<h1>
					<span id='directory'>
						$root_path
					</span>
					<span id='admin'>
						[$user@$hostname]
					</span>
				</h1>
			</div>
			<p>i dont care who are you, welcome to my own webserver :)</p>
		</header>
		<hr width='100%' size='1'>
	";
	
	echo "<ul class='main'>";
	foreach($list as $index => $value) {
		if(substr($req_path, -1) == "/") {
			echo "<li><u><a href='$req_path$value'>$value</a></u></li>";
		} else {
			echo "<li><u><a href='$req_path/$value'>$value</a></u></li>";
		}

	}
	echo "</ul>";
	echo "<hr width='100%' size='1'>"

	?>
</body>
</html>
