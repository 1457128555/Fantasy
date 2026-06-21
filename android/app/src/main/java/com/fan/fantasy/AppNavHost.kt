package com.fan.fantasy

import androidx.compose.runtime.Composable
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.toRoute

@Composable
fun AppNavHost() {
    val nav = rememberNavController()
    NavHost(navController = nav, startDestination = Home) {

        composable<Home> {
            HomeScreen(
                onPicked = { uri -> nav.navigate(Editor(uri = uri.toString())) }
            )
        }

        composable<Editor> { backStackEntry ->
            val editor: Editor = backStackEntry.toRoute()
            EditorScreen(
                uriString = editor.uri,
                onBack = { nav.popBackStack() }
            )
        }
    }
}
