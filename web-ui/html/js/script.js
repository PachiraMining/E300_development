function show_mode(input)
{   if (input.value == 1)
    {
        //document.getElementById("div_auto").style.display = "block";
        document.getElementById("div_manual").style.display = "none";
    }
    else
    {
        //document.getElementById("div_auto").style.display = "none";
        document.getElementById("div_manual").style.display = "block";                    
    }
}

function show_menu()
{
    document.getElementById("Table_Menu").style.visibility="visible";
}

function clear_color()
{
    document.getElementById("a_miner_setting").style.color = '#ffffff';
    document.getElementById("a_ip_setting").style.color = '#ffffff';
    document.getElementById("a_upfirm").style.color = '#ffffff';
    document.getElementById("a_password").style.color = '#ffffff';
    document.getElementById("a_dash_board").style.color = '#ffffff';
}

function click_submenu(index)
{   
    clear_color();
    if (index == 1)
    {
        document.getElementById("a_miner_setting").style.color = '#F5A623';
        //show_config();
    }
    else if (index == 2)
    {
        document.getElementById("a_ip_setting").style.color = '#F5A623';
    }
    else if (index == 3)
    {
        document.getElementById("a_upfirm").style.color = '#F5A623';                    
    }
    else if (index == 4)
    {
        document.getElementById("a_password").style.color = '#F5A623';                    
    }
    else if (index == 5)
    {
        document.getElementById("a_dash_board").style.color = '#F5A623';                    
    }
}

function    show_dashboard()
{
    
}

function show_config()
{
    document.getElementById("div_show_config").style.display = "block";
    document.getElementById("div_show_status").style.display = "none";
    document.getElementById("div_show_upfirmware").style.display = "none";
    document.getElementById("div_show_loadbitstream").style.display = "none";
}

function show_status()
{  
    document.getElementById("div_show_config").style.display = "none";
    document.getElementById("div_show_status").style.display = "block";
    document.getElementById("div_show_upfirmware").style.display = "none";
    document.getElementById("div_show_loadbitstream").style.display = "none";
}

function show_upfimrware()
{   
    document.getElementById("div_show_config").style.display = "none";
    document.getElementById("div_show_status").style.display = "none";
    document.getElementById("div_show_upfirmware").style.display = "block";
    document.getElementById("div_show_loadbitstream").style.display = "none";
}


function show_loadbit()
{
    
    document.getElementById("div_show_config").style.display = "none";
    document.getElementById("div_show_status").style.display = "none";
    document.getElementById("div_show_upfirmware").style.display = "none";
    document.getElementById("div_show_loadbitstream").style.display = "block";
}