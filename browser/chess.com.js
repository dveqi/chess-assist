// ==UserScript==
// @name         Chess Assist Browser Plugin (Chess.com)
// @namespace    http://tampermonkey.net/
// @version      0.1
// @description  hack the world!
// @author       dveqi
// @match        https://www.chess.com/
// @include      https://www.chess.com/*
// @icon         https://www.google.com/s2/favicons?domain=chess.com
// @grant        none
// ==/UserScript==

const ENV_TEST = 0;

const registeredDrawings = [];

const fileToN = (file) => {
  file = file.toUpperCase();
  const abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const index = abc.indexOf(file);
  return `${index + 1}`;
};

const gamePositionToBoardPosition = (pos) => {
  const left = fileToN(pos[0]);
  const right = pos[1];
  return `${left}${right}`;
};

const highlightSquare = (squareNotation) => {
  const board = getChessBoard();
  let highlightElement = document.createElement("div");
  let dataTestElement = document.createAttribute("data-test-element");
  dataTestElement.value = "highlight";
  highlightElement.setAttributeNode(dataTestElement);
  let classAttr = document.createAttribute("class");
  classAttr.value = `highlight square-${gamePositionToBoardPosition(
    squareNotation
  )}`;
  highlightElement.setAttributeNode(classAttr);
  let hStyle = document.createAttribute("style");
  hStyle.value = "background-color: rgb(162, 0, 124); opacity: 0.81337;";
  highlightElement.setAttributeNode(hStyle);
  //board.appendChild(highlightElement);
  board.prepend(highlightElement);
};

const clearHighlights = () => {
  const board = getChessBoard();
  // registeredDrawings.forEach(element => {
  //     console.log(element, board);
  //     board.removeChild(element);
  // });
  for (let div of board.getElementsByClassName("highlight")) {
    if (div.style.cssText.match(/opacity: 0.81337/)) board.removeChild(div);
  }
};

const getChessBoard = () => {
  const [board] = document.getElementsByClassName(
    ENV_TEST ? "layout-board board" : "board"
  );
  return board;
};

const getAPI = (board) => {
  const { game } = board;
  return game;
};
const timer = (ms) => new Promise((res) => setTimeout(res, ms));

let api = getAPI(getChessBoard());

let lastfen = "";

const getbestmove = () => {
  fetch(`http://127.0.0.1:8080/uci/bestmove`)
    .then((T) => T.json())
    .then((response) => {
      console.log("response best move: ", response);
      const { bestMove } = response;
      if (bestMove.length === 5) {
        const from = bestMove.substr(0, 2);
        const to = bestMove.substr(2, 4);
        highlightSquare(from);
        highlightSquare(to);
      } else if (bestMove.length === 4) {
        const from = bestMove.substr(0, 2);
        const to = bestMove.substr(2);
        highlightSquare(from);
        highlightSquare(to);
      }
      return response.bestMove;
    });
};
const updateposition = (pos) => {
  fetch(`http://127.0.0.1:8080/uci/position/?fen=${pos}`)
    .then((T) => T.json())
    .then((response) => {
      console.log("Position updated!");
      getbestmove();
    });
};

setInterval(() => {
  api = getAPI(getChessBoard());
  if (!api || api.getFEN().indexOf('8/8/8/8/8/8/8/8') != -1) return;
  if (api.getFEN() != lastfen) {
    clearHighlights();
    clearHighlights();
    lastfen = api.getFEN();
    if (lastfen) updateposition(lastfen);
  }
}, 10);

fetch("http://127.0.0.1:8080/uci/is_ready")
  .then((T) => T.json())
  .then((response) => {
    if (response.isReady) {
      console.log("Engine is ready to go!");
    }
  });
