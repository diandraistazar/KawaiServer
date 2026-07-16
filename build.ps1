param (
    [Parameter(Mandatory = $true)][ValidateSet("compile", "clean")][string]$Cmd
)

# Script Configuration
$Version="v1.0.0"
$SourceFiles = [System.Collections.ArrayList]@(
	"KawaiServer.java"
)
$SourceDir = ".\src"
$BuildDir = ".\build"
$Name = "KawaiServer.jar"
$EntryPoint = "KawaiServer" # KawaiServer.class as the entry point

### --[Helper functions]--

function Get-CmdExistence() {
    param (
        [string]$ExecName
    )

    foreach ($path in $Env:PATH.split(";")) {
        if (Test-Path $path\$ExecName) {
            return $True
        }
    }

    return $False
}

### --[Main logic of this script]--

Write-Host -ForegroundColor White "### BUILD SCRIPT FOR WINDOWS ###"
Write-Host -ForegroundColor White "## branch: $(git --no-pager branch --show-current)"
Write-Host -ForegroundColor White "## name: $Name"
Write-Host -ForegroundColor White "## ver: $Version"
Write-Host

if ($Cmd -eq "compile") {
    # Existence of javac, java, jar checking
    if (
        -not $(Get-CmdExistence("javac.exe")) -or
        -not $(Get-CmdExistence("java.exe")) -or
        -not $(Get-CmdExistence("jar.exe"))
    ) {
        Write-Host -ForegroundColor White -BackgroundColor Red "Is the java package installed?"
        exit 1
    }

	if (Test-Path $BuildDir) {
		Write-Host -ForegroundColor Yellow "$BuildDir directory is already exists. Re-creating"
	} else {
		New-Item $BuildDir -ItemType Directory | Out-Null
		Write-Host -ForegroundColor Yellow "New directory named $BuildDir created"
	}

	for ($i = 0; $i -lt $SourceFiles.Count; $i++) {
		$SourceFile = $SourceFiles[$i]
		$SrcFilePath = "$SourceDir\$SourceFile"
		$DestFilePath = "$BuildDir\$SourceFile"

		if(-not $(Test-Path $SrcFilePath)) {
			Write-Host -NoNewLine -ForegroundColor White "   $SrcFilePath -> $DestFilePath : "
			Write-Host -ForegroundColor Red "Not exists"
			exit 1
		}

		if ($SrcFilePath.EndsWith(".java")) {
			Write-Host -NoNewLine -ForegroundColor White "   $SrcFilePath -> $DestFilePath\$($SourceFile.Replace(".java", ".class")) : "
		
			$OutputStatus = javac $SrcFilePath -d $BuildDir -cp $SourceDir 2>&1
			if ($?) {
				Write-Host -ForegroundColor Green "Success"
    		} else {
        		Write-Host -ForegroundColor Red "Failed"
				Write-Host -ForegroundColor White "$OutputStatus" # Print the message error
				exit 1
			}
		}
		else {
			Write-Host -NoNewLine -ForegroundColor White "   $SrcFilePath -> $DestFilePath : "
			
			$LastSeperator = $DestFilePath.LastIndexOf("\")
			if ($LastSeperator -lt 0) {
				New-Item -Force -Type Directory $DestFilePath.Substring(0, $DestFilePath.Length) | Out-Null
			} else {
				New-Item -Force -Type Directory $DestFilePath.Substring(0, $LastSeperator) | Out-Null
			}

			try {
				Copy-Item -Recurse -Force $SrcFilePath $DestFilePath
				Write-Host -ForegroundColor Green "Copied"
			} catch [IOException] {
				Write-Host -NoNewLine -ForegroundColor Red "Copying failure"
			}

		}
	}
	
	Write-Host
	Set-Location $BuildDir
	jar -vcfe $Name $EntryPoint *
    if ($?) {
		Write-Host -ForegroundColor Green "Success to create the jar archive file from $BuildDir\*"
		Move-Item $Name .. -Force
		Set-Location ..
    } else {
		Write-Host -ForegroundColor Red "Failed to create the jar archive file from $BuildDir\*"
		Set-Location ..
        exit 1
	}
	
	Write-Host
	Write-Host -ForegroundColor Yellow "Compilation successfully :3"
	Write-Host -ForegroundColor White "try run ""java -jar $Name"""
} 

elseif ($Cmd -eq "clean") {
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host -ForegroundColor Yellow "$BuildDir directory removed"
    } else {
        Write-Host -ForegroundColor Yellow "$BuildDir directory isn't exists. SKIP"
    }

	if(Test-Path $Name) {
        Remove-Item -Recurse -Force $Name
        Write-Host -ForegroundColor Yellow "$Name removed"
	} else {
        Write-Host -ForegroundColor Yellow "$Name isn't exists. SKIP"
	}
} 

else {
    Write-Host -ForegroundColor Red "What the hell ""$($Cmd)"" command?"
    Write-host -ForegroundColor White "There are only ""compile"" and ""clean"" command"
    exit 1
}
